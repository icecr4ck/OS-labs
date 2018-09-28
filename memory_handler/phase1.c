#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { false, true } bool;
typedef struct Block Block;

struct Block{
	int offset;
	size_t size;
	bool is_free;
	Block *next;
	Block *prev;
};

typedef struct {
	Block *first_block;
	Block *last_alloc;
}MManager;

MManager *initmem(int max_size){
	MManager *mm = malloc(sizeof(*mm));;
	Block *b = malloc(sizeof(*b));
	b->offset = 0;
	b->size = max_size;
	b->is_free = true;
	b->next = NULL;
	b->prev = NULL;
	mm->first_block = b;
	mm->last_alloc = b;
	return mm;
}

int update_blocks(Block *b, Block *next_block, int size){
	b->offset = next_block->offset;
	b->size = size;
	b->is_free = false;
	next_block->size -= size;
	next_block->offset += size;
	return b->offset;
}

void clean_memory(MManager *mm){
	 Block *current = mm->first_block;
	 while (current != NULL){
		 if (!current->size){
			 if (current->prev == NULL){
				 mm->first_block = current->next;
			 }
			 else {
				 current->prev->next = current->next;
			 }
			 if (current->next != NULL){
				 current->next->prev = current->prev;
			 }
		 }
		 current = current->next;
	 }
}

int alloumem(MManager *mm, size_t size){
	int offset = -1;
	Block *b = malloc(sizeof(*b));
	Block *current = mm->first_block; 
	while (current != NULL){
		if ((current->is_free) && (current->size >= size)){
			// Met les blocs a jour
			offset = update_blocks(b, current, size);
			// Si on insert le bloc en tete de liste
			if (current->prev == NULL){
				b->prev = NULL;
				b->next = current;
				mm->first_block = b;
				current->prev = mm->first_block;
			}
			// Si on insert le bloc entre 2 autres blocs
			else {
				b->prev = current->prev;
				b->next = current;
				current->prev->next = b;
				current->prev = b;
			}
			mm->last_alloc = b;
			break;
		}
		current = current->next;
	}
	clean_memory(mm);
	if (offset == -1){
		printf("ERREUR: impossible d'allouer un bloc de taille %zu par manque de place.\n", size);
	}
	return offset;
}

void liberemem(MManager *mm, int offset){
	Block *current = mm->first_block;
	while (current != NULL){
		if ((!current->is_free) && (current->offset == offset)){
			current->is_free = true;
			// Si les blocs adjacents ne sont pas nuls
			if (current->prev != NULL && current->next != NULL){
				// Si les blocs adjacents sont libres
				if (current->prev->is_free && current->next->is_free){
					current->prev->size += current->size + current->next->size;
					current->prev->next = current->next->next;
					if (current->next->next != NULL){
						current->next->next->prev = current->prev;
					}
				}
				// Si seulement le bloc precedent est libre
				else if (current->prev->is_free && !current->next->is_free){
					current->prev->size += current->size;
					current->prev->next = current->next;
					current->next->prev = current->prev;
				}
				// Si seulement le bloc suivant est libre
				else if (!current->prev->is_free && current->next->is_free){
					current->size += current->next->size;
					current->next = current->next->next;
					if (current->next->next != NULL){
						current->next->next->prev = current;
					}
				}
			}
			// Si seulement le bloc suivant n'est pas nul
			else if (current->prev == NULL && current->next != NULL){
				if (current->next->is_free){
					current->size += current->next->size;
					current->next = current->next->next;
					if (current->next->next != NULL){
						current->next->next->prev = current;
					}
				}
			}
			// Si seulement le bloc precedent n'est pas nul
			else if (current->prev != NULL && current->next == NULL){ 
				if (current->prev->is_free){
					current->prev->size += current->size;
					current->prev->next = current->next;
				}
			}
			break;
		}
		current = current->next;
	}
}

int nbloclibres(MManager *mm){
	int nb_blocks_free = 0;
	Block *current = mm->first_block;
	while (current != NULL){
		if (current->is_free){
			nb_blocks_free++;
		}
		current = current->next;
	}
	return nb_blocks_free;
}

int nblocalloues(MManager *mm){
	int nb_blocks_alloc = 0;
	Block *current = mm->first_block;
	while (current != NULL){
		if (!current->is_free){
			nb_blocks_alloc++;
		}
		current = current->next;
	}
	return nb_blocks_alloc;
}

int memlibre(MManager *mm){
	int free_mem = 0;
	Block *current = mm->first_block;
	while (current != NULL){
		if (current->is_free){
			free_mem += current->size;
		}
		current = current->next;
	}
	return free_mem;
}

int mem_pgrand_libre(MManager *mm){
	int max_block = 0;
	Block *current = mm->first_block;
	while (current != NULL){
		if (current->is_free && current->size > max_block){
			max_block = current->size;
		}
		current = current->next;
	}
	return max_block;
}

int mem_small_free(MManager *mm, int maxTaillePetit){
	int nb_small_blocks = 0;
	Block *current = mm->first_block;
	while (current != NULL){
		if (current->is_free && current->size < maxTaillePetit){
			nb_small_blocks++;
		}
		current = current->next;
	}
	return nb_small_blocks;
}

bool mem_est_alloue(MManager *mm, int pOctet){
	bool res;
	int start, end;
	Block *current = mm->first_block;
	while (current != NULL){
		start = current->offset;
		end = start + (current->size-1);
		if (pOctet >= start && pOctet <= end){
			res = !current->is_free;
			break;
		}
		current = current->next;
	}
	return res;
}

void show_memory(MManager *mm){
	int i = 1;
	Block *current = mm->first_block;
	while (current != NULL){
		printf("--Block %d--\n",i);
		printf("\tOffset: %d\n",current->offset);
		printf("\tTaille: %zu\n",current->size);
		if (current->is_free) printf("\tLibre: oui\n");
		else printf("\tLibre: non\n");
		current = current->next;
		i++;
	}
}

void show_infos(MManager *mm, int size_min, int pOctet){
	printf("\n=======Informations sur l'état de la mémoire=======\n");
	printf("Nombre de blocs libres plus petit que %d: %d\n", size_min, mem_small_free(mm, size_min));
	printf("Nombre de blocs libres: %d\n", nbloclibres(mm));
	printf("Nombre de blocs alloués: %d\n", nblocalloues(mm));
	printf("Mémoire libre: %d\n", memlibre(mm));
	printf("Taille du plus grand bloc libre: %d\n", mem_pgrand_libre(mm));
	if (mem_est_alloue(mm, pOctet)){
		printf("L'offset %d est alloué.\n", pOctet);
	}
	else {
		printf("L'offset %d n'est pas alloué.\n", pOctet);
	}
}


int main(int argc, char *argv[]){
	int offset[100] = {0};
	if (argc != 2){
		printf("Syntaxe: ./lab3 memory_size\n");
		exit(1);
	}
	int mem_size = atoi(argv[1]);

	// Initialisation du gestionnaire de memoire
	MManager *mm = initmem(mem_size);

	// Affiche l'etat initial de la memoire
	printf("=======Etat initial========\n");
	show_memory(mm);

	// Alloue 5 blocs de taille 25, 55, 5, 10, 35
	printf("\n=======Allocation de 5 blocs de taille 25, 55, 5, 10 et 35=======\n");
	offset[0] = alloumem(mm, 25);
	offset[1] = alloumem(mm, 55);
	offset[2] = alloumem(mm, 5);
	offset[3] = alloumem(mm, 10);
	offset[4] = alloumem(mm, 35);
	
	// Affiche l'etat de la memoire apres allocation
	show_memory(mm);

	// Libere les blocs 1, 3 et 4
	printf("\n=======Libération des blocs 1, 3 et 4=======\n");
	liberemem(mm, offset[0]);
	liberemem(mm, offset[2]);
	liberemem(mm, offset[3]);
	
	// Afficher l'etat de la memoire apres la liberation des blocs
	show_memory(mm);

	// Alloue 2 nouveaux blocs de taille 15 et 40
	printf("\n=======Allocation de 2 blocs de taille 15 et 40=======\n");
	offset[5] = alloumem(mm, 15);
	offset[6] = alloumem(mm, 40);

	// Affiche l'etat de la memoire apres allocation
	show_memory(mm);

	// Libere le bloc 2
	printf("\n=======Libération du bloc de taille 55=======\n");
	liberemem(mm, offset[1]);

	// Affiche l'etat de la memoire apres la liberation du bloc
	show_memory(mm);

	// Alloue un bloc de taille 5
	printf("\n=======Allocation de 1 bloc de taille 5=======\n");
	offset[7] = alloumem(mm, 5);

	// Affiche l'etat de la memoire apres allocation
	show_memory(mm);

	// Alloue un bloc de taille 20
	printf("\n=======Allocation de 1 bloc de taille 20=======\n");
	offset[7] = alloumem(mm, 20);

	// Affiche l'etat de la memoire apres allocation
	show_memory(mm);

	show_infos(mm, 80, 130);
	return 0;
}
