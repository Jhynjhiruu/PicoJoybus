bin2header -o data.tmp -n controller_pak "$1"
sed -e 's/const //' data.tmp > src/data.h
rm data.tmp