cat kernel.loc.cpp | grep loc: | cut -d: -f2 | cut -d* -f1 | awk 'BEGIN{x=0}{x = x+$1}END{print x}'
