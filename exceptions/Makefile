##### System dependent options

CXX      = g++
LINK     = g++
CXXFLAGS = 
LFLAGS   =
LIBS     =


##### Target

TARGET   = thrower


##### Files

HEADERS  = thrower.h
SOURCES  = thrower.cc \
           main.cc
OBJECTS  = thrower.o \
           main.o


##### Implicit rules

.cc.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<


##### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)

clean:
	-rm -f *~ core
	-rm -f $(OBJECTS)


##### Compile

thrower.o: thrower.cc \
           thrower.h

main.o:    main.cc

