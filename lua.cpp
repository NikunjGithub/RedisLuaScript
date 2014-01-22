#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <hiredis/hiredis.h>
#include <boost/thread/thread.hpp>

using namespace std;

struct timeval _timeout ;
redisContext *_redisContext;
const long long SEC_TO_USEC = 1000000 ;

void connect(const std::string &ip,
		int port,
		int timeoutInUsec )
{
	_timeout.tv_sec = timeoutInUsec / SEC_TO_USEC ;
	_timeout.tv_usec = timeoutInUsec % SEC_TO_USEC ;

	_redisContext = redisConnectWithTimeout(ip.c_str(), port, _timeout);
	if (_redisContext->err)
	{
		std::cout << "Cannot connect to redis server. "
			<< " Error : " << _redisContext->errstr
			<< std::endl ;
		exit(1);
	}
}

string scriptSingleCommand = "local link_id = redis.call(\"INCR\", KEYS[1]) "
"return link_id ";

string scriptMultipleCommands =
"local link_id = redis.call(\"INCR\", KEYS[1])      	"
"redis.call(\"HSET\", KEYS[2], link_id, ARGV[1])    	"
"local data = redis.call(\"HGETALL\",KEYS[2])       	"
"return data                                        	";

void eval()
{
	redisReply *reply =
		( redisReply * ) redisCommand( _redisContext,
				"eval %s 1 %s", "return {KEYS[1]}", "key1") ;

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (int j = 0; j < reply->elements; j++) {
			cout<< j <<","<<reply->element[j]->str<<endl;
		}
	}
	cout<<"EVAL: "<< reply->str<<endl;
	freeReplyObject(reply);
}

void evalSingleCommand()
{
	string command;
	command.append( scriptSingleCommand );
	redisReply *reply =
		( redisReply * ) redisCommand( _redisContext,
				"EVAL %s %d %s ",command.c_str(),1,"a");

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (int j = 0; j < reply->elements; j++) {
			cout<< j <<","<<reply->element[j]->str<<endl;
		}
	}
	cout<<endl<<"EVAL: "<< reply->str<<endl;
	freeReplyObject(reply);
}

void evalMultipleCommands(char** argv)
{
	string command;
	command.append( scriptMultipleCommands );
	redisReply *reply =
		( redisReply * ) redisCommand( _redisContext,
				"EVAL %s %d %s %s %s",command.c_str(),2,argv[1],argv[2],argv[3]);

	cout<<"Redis reply type "<<reply->type<<endl;

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		cout<<"Redis reply size "<<reply->elements<<endl;
		for (int j = 0; j < reply->elements; j++)
		{
			if((j+1) < reply->elements)
			{
				cout<<(reply->element[j]->str)<<","<<(reply->element[j+1]->str)<<endl;
				++j;
			}
		}
	}
	else if (reply->type == REDIS_REPLY_INTEGER) {
		cout<<"Key value "<<reply->integer<<endl;
	}
	else
		cout<<endl<<"EVAL: "<< reply->str<<endl;

	freeReplyObject(reply);
}

int main(int argc,char** argv)
{
	connect("10.0.0.30",6379,1500000);
	//  eval();
	//  evalSingleCommand();
	evalMultipleCommands(argv);

	return 0;
}

