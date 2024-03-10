for file in *.c
  do
    echo $file
    name=${file%.*}
    gcc -o ${name}.$1 $file && mkdir build && mv ${name}.$1 build/${name}.$1
  done