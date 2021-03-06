#!/bin/sh
#

aclocal -I m4
# Generate 'src/vsip/impl/acconfig.hpp.in' by inspecting 'configure.ac'
autoheader
# Generate 'configure' from 'configure.ac'
autoconf

# Tell configure to ignore non-executable/object files generated by
# greenhills compiler
cat configure | sed -e "s,\*.xcoff,\*.xcoff | *.dla | *.dnm | *.dbo | *.map," > configure.tmp
mv configure.tmp configure
chmod +x configure

if test -f "vendor/fftw/configure.ac"; then
  cd vendor/fftw
  aclocal -I m4
  autoheader
  autoconf
  cd ../..
fi
