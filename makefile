bmain = ./

run :
	${bmain}/sharepos 

stop :
	${bmain}/sharepos stop

stop_s : 
	kill -SIGINT ${pid}
	top



