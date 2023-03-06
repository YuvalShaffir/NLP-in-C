tweets: tweets_generator.c markov_chain.c markov_chain.h linked_list.c linked_list.h
	gcc	-Wall -Wextra -Wvla -std=c99 tweets_generator.c markov_chain.c linked_list.c -o tweets_generator

snake: snakes_and_ladders.c markov_chain.c markov_chain.h linked_list.c linked_list.h
	gcc	-Wall -Wextra -Wvla -std=c99 snakes_and_ladders.c markov_chain.c linked_list.c -o snakes_and_ladders