TARGET = SaaJ-bash
SOURCE = SaaJ-bash.c
all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCE)
	gcc -Wall $(SOURCE) -o $(TARGET)

debug: $(SOURCE)
	gcc -g -Wall $(SOURCE) -o $(TARGET)

clean:
	rm -f $(TARGET)	