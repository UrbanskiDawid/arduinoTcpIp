
#!/bin/bash
echo "clean"
rm a.out -f 2> /dev/null
echo "compile"

g++ main.cpp -std=c++0x -lrt -lpthread

if [ $? -eq 0 ]
then
	echo "commit"	
	#git commit -m "-kopilacja-"
	#git push origin master

	echo "run"
	./a.out
else
	echo "stop "
	exit 1
fi
