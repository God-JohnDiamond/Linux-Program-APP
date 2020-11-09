objs := main.o sub.o

test: $(objs)
	gcc -o test $^

dep_files := $(foreach f, $(objs), .$(f).d)
dep_files := $(wildcard $(dep_files))

ifneq ($(dep_files),)
	include $(dep_files)
endif

%.o: %.c
	gcc -Wp,-MD,.$@.d -c -o $@ $<

clean:
	rm *.o test -f

distclean:
	rm $(dep_files) *.o test -f
