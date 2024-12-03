.PHONY: all setup run-stest run-mtest

all: setup run-stest run-mtest

setup:
	mkdir -p tmp/mtacoin
	echo "difficulty=24" > tmp/mtacoin/config.txt
        sudo docker pull royhz/mta_blockchain:mtacoin-server
        sudo docker pull royhz/mta_blockchain:mtacoin-miner


run-stest:
	sudo docker run -d --rm -v $(shell pwd)/tmp/mtacoin:/mnt/mta royhz/mta_blockchain:mtacoin-server

run-mtest:
	for i in 1 2 3; do \
		sudo docker run -d --rm -v $(shell pwd)/tmp/mtacoin:/mnt/mta royhz/mta_blockchain:mtacoin-miner; \
	done

