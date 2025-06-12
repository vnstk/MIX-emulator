# Vainstein K 2025may24

CPP=g++

CXXFLAGS= -fcheck-new -fcoroutines -fdiagnostics-path-format=inline-events -fdiagnostics-show-template-tree -fdiagnostics-text-art-charset=unicode -fno-exceptions -fno-diagnostics-show-cwe -fno-modules -fno-operator-names -grecord-gcc-switches -static -static-libgcc -static-libstdc++ -Wformat=1 -Wformat-overflow=1 -Wformat-truncation=1 -Wno-class-conversion -Wno-endif-labels -Wno-misleading-indentation -Wno-system-headers -Wno-old-style-cast -Wno-parentheses -Wno-xor-used-as-pow -Warray-bounds=1 -Warray-parameter -Wattribute-alias=2 -Wclobbered -Wdangling-else -Wdangling-pointer=2 -Wdangling-reference -Wdeprecated-copy -Wduplicated-branches -Wduplicated-cond -Wfloat-equal -Wfree-nonheap-object -Winfinite-recursion -Winit-self -Wmismatched-dealloc -Wmismatched-new-delete -Wmissing-include-dirs -Wmissing-template-keyword -Wmultistatement-macros -Wnoexcept-type -Wnrvo -Wodr -Woverloaded-virtual -Wpacked-not-aligned -Wpessimizing-move -Wrange-loop-construct -Wredundant-move -Wreorder -Wrestrict -Wreturn-local-addr -Wself-move -Wsequence-point -Wshadow=local -Wsizeof-array-div -Wsizeof-pointer-div -Wstring-compare -Wstringop-overflow=2 -Wtautological-compare -Wtrigraphs -Wtype-limits -Wuse-after-free=2 -Og -std=c++20

EXE=emu

$(EXE) : globals.o core.o arith.o main.o
	$(CPP) $(CXXFLAGS) $^ -o $@ 

%.o : %.cpp
	$(CPP) $(CXXFLAGS) -fPIC -c $< -o $@

clean :
	rm -fv $(EXE) *.o *.ii

# refresher:
#			$@		---		tgt name, full.
#			$*		---		tgt name sans file extension.
#			$<		---		name of first (Lmost-listed) prereq
#			$^		---		names of all the prereqs, space-sep
#
# Much more, at https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
