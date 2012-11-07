all: siteanalyzer

siteanalyzer: main.o crawler.o webgrap.o urlhash.o urlqueue.o link_parser.o http_client.o network.o
	cc -g -o siteanalyzer main.o crawler.o webgrap.o urlhash.o urlqueue.o link_parser.o http_client.o network.o -lpthread

main.o: main.c crawler.h webgrap.h
	cc -g -o main.o -c main.c

crawler.o: crawler.c crawler.h webgrap.h urlqueue.h link_parser.h http_client.h network.h
	cc -g -o crawler.o -c crawler.c -lpthread

webgrap.o: webgrap.c webgrap.h urlhash.h
	cc -g -o webgrap.o -c webgrap.c

urlhash.o: urlhash.c urlhash.h
	cc -g -o urlhash.o -c urlhash.c

urlqueue.o: urlqueue.c urlqueue.h
	cc -g -o urlqueue.o -c urlqueue.c

link_parser.o: link_parser.c link_parser.h
	cc -g -o link_parser.o -c link_parser.c

http_client.o: http_client.c http_client.h
	cc -g -o http_client.o -c http_client.c

network.o: network.c network.h
	cc -g -o network.o -c network.c
