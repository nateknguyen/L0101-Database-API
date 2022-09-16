# ---------------------------------------
# Program Specific Defines
# ---------------------------------------
CC := g++
TARGET := demo_main
TEST_TARGET := test_main
LIBRARIES := 
DEFINITIONS := 
FLAGS = -g -std=c++17 -Wall -Werror -fopenmp


# ---------------------------------------
# Probably don't need to change
# ---------------------------------------

# Directory layout
SDIR := source
ODIR := $(SDIR)/obj
IDIR := include
DDIR := $(ODIR)/.deps
TDIR := test
TODIR := $(TDIR)/obj
TDDIR := $(TODIR)/.deps

# create flags
LIBS = $(patsubst %, -l%, $(LIBRARIES))
DEFS = $(patsubst %, -D%, $(DEFINITIONS))
IPATHS = $(patsubst %, -I%, $(IDIR))
CFLAGS = $(FLAGS) $(IPATHS) $(DEFS) $(LIBS)

TEST_FLAGS = $(filter-out -lpthread, $(filter-out -g, $(CFLAGS))) -lgtest -lgtest_main -lpthread

# get source and object file names
SOURCE = $(notdir $(wildcard $(SDIR)/*.cpp))
OBJ = $(patsubst %.cpp, $(ODIR)/%.o, $(SOURCE))
TESTS = $(wildcard $(TDIR)/*.cpp)

TSOURCE = $(notdir $(wildcard $(TDIR)/*.cpp))
TOBJ = $(patsubst %.cpp, $(TODIR)/%.o, $(TSOURCE))

# compilation and linking
$(ODIR)/%.o : $(SDIR)/%.cpp
$(ODIR)/%.o : $(SDIR)/%.cpp $(DDIR)/%.d | $(DDIR)
	$(CC) -c -o $@ $< $(CFLAGS) -MT $@ -MMD -MP -MF $(DDIR)/$*.d

$(TODIR)/%.o : $(TDIR)/%.cpp
$(TODIR)/%.o : $(TDIR)/%.cpp $(TDDIR)/%.d | $(TDDIR)
	$(CC) -c -o $@ $< $(TEST_FLAGS) -MT $@ -MMD -MP -MF $(TDDIR)/$*.d

$(TARGET) : $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

test : $(filter-out $(patsubst %.cpp, $(ODIR)/%.o, main.cpp), $(OBJ)) $(TOBJ)
	$(CC) -o $(TEST_TARGET) $^ $(TEST_FLAGS)
	./$(TEST_TARGET)

gen_data : datasets/gen_data.cpp
	g++ -o gen_data $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(DDIR)/.d $(TARGET) $(TEST_TARGET)

setup:
	mkdir -p $(SDIR) $(IDIR) $(ODIR) $(DDIR) $(TDIR) $(TSDIR)

$(DDIR): ; @mkdir -p $@

DEPFILES = $(patsubst %.cpp, $(DDIR)/%.d, $(SOURCE)) $(patsubst %.cpp, $(TDDIR)/%.d, $(TSOURCE))
$(DEPFILES):

include $(wildcard $(DEPFILES))
