#include $(targ).inc

### COMPILER AND LINKER FLAGS ########################################
ldflags += -lm -lc -lgcc -pthread -Wl,-Map=output.map

# Compiler flag for debugging purposes
ifdef DEBUG
   ifeq ($(DEBUG),1)
       cflags  += -g
       ccflags += -g
       ldflags += -g
   endif
else
   cflags +=
   ccflags +=
   ldflags +=
endif

cflags  += -Wconversion -Wextra  -Wsign-conversion
# cflags  += -Werror=sign-conversion
# cflags  += -Wall -Werror -Wextra

######################################################################

### CREATING AN OBJ DIRECTORY ########################################
ifndef $(OBJDIR)
	OBJDIR=./bin
endif
#### Creating an output directory
$(shell mkdir $(OBJDIR))

######################################################################

### RULES ############################################################
ifeq ($(compiler),GNUCOMPILER)
%.$(lib):
	echo ARCHIVING $@
	@ar ru $@ $^

$(OBJDIR)/%.$(obj): %.c
	@echo COMPILING $<
	@gcc	$(cflags) $(defines:%=-D%) $(includes:%=-I%) -c $<	-o $(OBJDIR)/$(notdir $@)

$(OBJDIR)/%.$(obj): %.cpp
	@echo cpp COMPILING $<
	@gcc	$(cflags) $(defines:%=-D%) $(includes:%=-I%) -c $<	-o $(OBJDIR)/$(notdir $@)

%.$(exe):
	@echo LINKING $@
	@g++ $(ldflags) -o $(notdir $@) $^

endif #	GNUCOMPILER

#######################################################################

#### GENERIC STUFF ####################################################


# Make the web page	files
#%.c %.h: %.htm	%.cla
#	echo BINTOC	$<
#	../webpage/bintoc $(subst /,\\,$<) $(subst /,\\,$(basename $(notdir	$<)))

# Make dependency files
%.d: %.c
	echo UPDATING DEPENDENCIES FOR $<
	gcc	-undef -M $< $(defines:%=-D%) $(builtins:%=-D%)	$(includes:%=-I%) |	depend $(obj) >	$@

.INTERMEDIATE: $(modules:%=%.d)
$(targ).inc: $(modules:%=%.d)
	echo MAKING	$@
	cat	*.d	> $@

### CLEANING ###########################################################
.PHONY:	CLEAN
CLEAN:
	echo CLEANING
	rm -rf	$(OBJDIR)/*
	rm -rf   *.o
	rm -rf	*.a
	rm -rf   *.out

.PHONY:	clean
clean:
	echo CLEANING
	rm -rf	$(OBJDIR)/*
	rm -rf   *.o
	rm -rf	*.a
	rm -rf   *.out


.PHONY:	dep
dep:
	echo DEPENDENCIES UPDATED

###################### THE END ##########################################
