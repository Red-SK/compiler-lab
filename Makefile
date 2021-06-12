CC = g++
TARGET = main
CFLAGS = -I tokenizer -I util -I parser -I translator -I ./ -O2
DIRS = tokenizer util parser translator ./  
CFILES = $(foreach dir, $(DIRS),$(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp,%.o,$(CFILES)) 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $^ $(CFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $^ $(CFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)