for file in *.c
  do
    echo $file
    name=${file%.*}
    gcc -o ${name}-c.$1 $file && mv ${name}-c.$1 build/${name}-c.$1
  done