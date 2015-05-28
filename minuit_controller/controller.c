#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include "controller.h"

struct node{
	char * name;
	char * description;
	char * * attributes;
};

Node new_node(char * name, char * description, char ** attributes){
	Node n = malloc(sizeof(struct node));
	n->name = malloc(strlen(name)+1);
	strcpy(n->name, name);
	n->description = malloc(strlen(description)+1);
	strcpy(n->description, description);
	n->attributes = malloc(sizeof(attributes));
/*
	for(int i = 0 ; i < sizeof(attributes) ; i++){
		n->attributes[i] = malloc(sizeof(strlen(attributes[i])));
		strcpy(n->attributes[i], attributes[i]);
	}*/
	return n;

}

Metabot new_metabot(char ** t_moves, char ** t_sys, char ** t_modes){
	Metabot m = malloc(NB_NODES*sizeof(Node));
	m[0]=new_node("moves", "Fonctions de mouvement", t_moves);
	m[1]=new_node("sys", "Fonctions système", t_sys);
	m[2]=new_node("modes", "Fonctions modes", t_modes);
	return m;
}

void free_node(Node n){
	free(n->name);
	free(n->description);
	/*
	for(int i = 0 ; i < sizeof(n->attributes) ; i++){
		free(n->attributes[i]);
	}
	free(n->attributes);*/
}

void free_metabot(Metabot m){
	for(int i = 0; i < NB_NODES ; i++)
		free_node(m[i]);
	free(m);
}

void display_node(Node n){
	printf("%s : { ", n->name);
	for(int j = 0 ; j < sizeof(n->attributes) ; j++){
		printf("%s " , n->attributes[j]);
	}
	printf("}\n");
}

void display_metabot(Metabot m){
	printf("Metabot :\n");
	for(int i = 0 ; i < NB_NODES ; i++){
		display_node(m[i]);
	}
}


int size_bytes(char * s)
{
    if(strlen(s)%4 == 0)
        return strlen(s);
    else
        return  strlen(s) + 4 - strlen(s)%4;
}

bool cmp_name_node(char * name, Node n){
	return strcmp(name, n->name);
}

int write_data(UDPpacket *p, char * s, int it)
{
    for(int j = 0 ; j < size_bytes(s) ; j++)
    {
        if(j < strlen(s))
            p->data[it] = s[j];
        else
            p->data[it] = 0;
        it++;
    }
    return it;
}

void receive_OSC_command(UDPpacket *p, int fd)
{
    //Réception d'un message OSC standard
    char * name = malloc(sizeof(*name)*32);
    int i = 0;
    //comptage du nombre de caractères de la commande
    while(p->data[i]!=',')
    {
        i++;
    }
    //copie de la commande
    name = strncpy(name, (char *)p->data+1, i);

    //la séquence est elle terminée ?
    i++;
    //type de l'argument
    char type = p->data[i];
    i+=3;

    //copie de l'argument
    float val=0;
    if(type == 'f')
    {
        long int * d = malloc(sizeof(long int));
        for(int j = 0; j < 4 ; j++)
        {
            *d <<= 8;
            *d += p->data[i+j];
        }
        float * f = (float * )d;
        val = *f;
        free(d);
    }
    if(type == 'i')
    {
        long int data = 0;
        for(int j = 0; j < 4 ; j++)
        {
            data <<= 8;
            data += p->data[i+j];
        }
        val = (float) data;
    }

    //concaténation de la commande
    sprintf( name, "%s %f\n", name, val );

    //affichage de la commande envoyée dans le terminal
    printf("%s \n", name);
    //envoi de la commande
    if(!DEBUG_MODE){
    	if(write(fd, name, strlen(name)) == -1)
    		printf("Couldn't write \"%s\"", name);
    }

    free(name);
}

void send_answer(char ** cmd, int port)
{
    UDPsocket sd;
    IPaddress srvadd;
    UDPpacket *p;

    /* Open a socket on random port */
    if (!(sd = SDLNet_UDP_Open(0)))
    {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Resolve server name  */
    if (SDLNet_ResolveHost(&srvadd, "localhost", port) == -1)
    {
        fprintf(stderr, "SDLNet_ResolveHost(%s %d): %s\n", "localhost", port, SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Allocate memory for the packet */
    if (!(p = SDLNet_AllocPacket(512)))
    {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    int size = 0;
    int it = 0;
    for(int i_cmd = 0 ; i_cmd < 9 ; i_cmd ++)
    {
        size += size_bytes(cmd[i_cmd]);
        it = write_data(p, cmd[i_cmd], it);
    }

    p->address.host = srvadd.host;	/* Set the destination host */
    p->address.port = srvadd.port;	/* And destination port */

    p->len = size;
    SDLNet_UDP_Send(sd, -1, p); /* This sets the p->channel */
    SDLNet_FreePacket(p);
    SDLNet_Quit();
}

void answer_namespace(int port)
{
    char * cmd[] = { "Metabot" , "namespace" , ",s" , "/", "nodes={" , "moves", "sys", "modes", "}" };
    send_answer(cmd, port);
}

void answer_namespace_node( int x, int port)
{
}

