.IGNORE: all


all:
	~/script/TimeStamp.sh
	python ./gen_mono_bft.py  1>>./out.txt 2>>./error.txt
	write ylxiao pts/4 < ./out.txt
	write ylxiao pts/4 < ./error.txt
	rm -rf ./out.txt
	rm -rf ./error.txt
clean:

	rm -rf ./host
