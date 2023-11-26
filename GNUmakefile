CXXFLAGS += -I. $(shell root-config --cflags) -g
LDFLAGS += $(shell root-config --libs) -lPhysics -lMatrix -g

PROGRAMS = SCEStudyDx SCEStudyDyDz cathodeDeflection

all:	clean $(PROGRAMS)

$(PROGRAMS):
	@echo '<<compiling' $@'>>'
	@$(CXX) $@.cpp -o $@ $(CXXFLAGS) $(LDFLAGS)
	@rm -rf *.dSYM
clean:	
	rm -f $(PROGRAMS)
