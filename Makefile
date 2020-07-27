CXX = g++
CXXFLAGS = -Wall -std=c++11
NVCC = nvcc
NVCCFLAGS = -std=c++11

#this is setted for Quadro P2200
ARCH = -gencode arch=compute_61,code=compute_61

#use default cuda path
CUDA_PATH = /usr/local/cuda
CUDA_INCLUDE = $(CUDA_PATH)/include
OPENCV_PATH = /home/nemo/software/environment/opencv-4.4
OPENCV_INCLUDE = $(OPENCV_PATH)/include/opencv4
THIRD_PATH = ./thirdinclude
THIRD_INCLUDE = $(wildcard $(THIRD_PATH)/*)
INCLUDE = $(addprefix -I,$(CUDA_INCLUDE) $(THIRD_INCLUDE) $(THIRD_PATH) $(OPENCV_INCLUDE))
CUDA_LIB = $(CUDA_PATH)/lib64
OPENCV_LIB = $(OPENCV_PATH)/lib
ENGINE_LIB = ./libs
LIB_PATH = $(addprefix -L,$(CUDA_LIB) $(ENGINE_LIB) $(OPENCV_LIB))
LD_LIB_PATH= -Wl,-rpath=$(ENGINE_LIB)

OPENCV_LIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_dnn -lopencv_dnn_objdetect -lopencv_imgcodecs -lopencv_videoio
CUDA_LIBS = -lcudart -lcublas -lcurand -lcudnn
LINK_LIBS = $(OPENCV_LIBS) $(CUDA_LIBS) -lglog -lgflags

#compiler set
SRC_PATH = src
INTERFACE_PATH = interface
TEST_PATH = test

#set engine exe name
EXE_NAME = OpencvYolo
TEXT_NAME = $(addprefix Test,$(EXE_NAME))
DEBUG = 0
OPENMP = 0
THREAD = 1

ifeq ($(DEBUG),1)
CXXFLAGS += -g
else
CXXFLAGS += -Ofast
endif

ifeq ($(OPENMP),1)
CXXFLAGS += -fopenmp
endif

ifeq ($(THREAD),1)
CXXFLAGS += -pthread
endif

SRC_MODULE = $(wildcard $(SRC_PATH)/*)
C_SRC := $(foreach x,$(SRC_MODULE),$(wildcard $(x)/*.c))
C_OBJS:= $(patsubst %.c,%.o,$(C_SRC))

CPP_SRC := $(foreach x,$(SRC_MODULE),$(wildcard $(x)/*.cpp))
CPP_OBJS:= $(patsubst %.cpp,%.o,$(CPP_SRC))

CUDA_SRC := $(foreach x,$(SRC_MODULE),$(wildcard $(x)/*.cu))
CUDA_OBJS:= $(patsubst %.cu,%.o,$(CUDA_SRC))

OBJS := $(C_OBJS) $(CPP_OBJS) $(CUDA_OBJS)

#for engine
INTERFACE_SRC := $(wildcard $(INTERFACE_PATH)/*.cpp)
INTERFACE_OBJ := $(patsubst %.cpp,%.o,$(INTERFACE_SRC))

#for test
TEST_SRC := $(wildcard $(TEST_PATH)/*.cpp)
TEST_OBJ := $(patsubst %.cpp,%.o,$(TEST_SRC))


ifeq ($(words $(INTERFACE_OBJ)),1)
TARGET = $(EXE_NAME)
endif

ifeq ($(words $(TEST_OBJ)),1)
TARGET += $(TEXT_NAME)
endif

.PHONY:all
all : $(TARGET)

$(EXE_NAME):$(OBJS) $(INTERFACE_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIB_PATH) $(LD_LIB_PATH) $(LINK_LIBS)
$(TEXT_NAME):$(OBJS) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIB_PATH) $(LD_LIB_PATH) $(LINK_LIBS)

#depends
%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
%.o : %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
%.o : %.cu
	$(NVCC) $(ARCH) $(NVCCFLAGS) $(INCLUDE) -c $< -o $@

.PHONY:Test
Test:
	@echo $(INCLUDE)
	@echo $(LIB_PATH)
	@echo $(LD_LIB_PATH)
	@echo $(SRC_MODULE)
	@echo $(CPP_SRC) $(CUDA_SRC) $(C_SRC)
	@echo $(INTERFACE_SRC)
	@echo $(TEST_SRC)
	@echo $(OBJS)
	@echo  $(INTERFACE_OBJ) $(words $(INTERFACE_OBJ))

.PHONY:clean
clean:
	@rm -f $(OBJS) $(INTERFACE_OBJ) $(TEST_OBJ) $(EXE_NAME) $(TEXT_NAME)
	@rm -rf release

.PHONY:install
install:
	@mkdir -p ./release
	@cp ${EXE_NAME} ./release
	@cp -r models ./release
	@cp -r libs ./release

