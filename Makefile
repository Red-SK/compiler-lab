CC = g++
TARGET = main
CFLAGS = -I tokenizer -I util -I parser -I ./  
DIRS = tokenizer util parser ./  
CFILES = $(foreach dir, $(DIRS),$(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp,%.o,$(CFILES)) 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $^ $(CFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $^ $(CFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)