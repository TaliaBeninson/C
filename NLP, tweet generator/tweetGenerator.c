#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_WORD_LENGTH 100
#define MAX_SENTENCE_LENGTH 1000

typedef struct WordStruct {
    char *word;//parameter to save the word
    struct WordProbability *prob_list;//dynamic array to save the words +WordStruct of the next words
    int lengthOfProbList;//parameter to save amount of words that can be after the word
    int occurrence;//parameter to save the number of times a word exists in file

} WordStruct;

typedef struct WordProbability {
    struct WordStruct *word_struct_ptr;//saves the word+array of next words
    int occurrenceInProbList;//parameter to save for every word in array how many times the word appeared after the word

} WordProbability;

/**** LINKED LIST ****/
typedef struct Node {
    WordStruct *data;
    struct Node *next;
} Node;

typedef struct LinkList {
    Node *first;
    Node *last;
    int size;
} LinkList;

int add(LinkList *link_list, WordStruct *data);
void fill_dictionary(FILE *fp, int words_to_read, LinkList *dictionary);
WordStruct* getOrAddNode(LinkList *dictionary, char *word);
int add_word_to_probability_list(WordStruct *first_word,WordStruct *second_word);
void print( LinkList *dictionary);
int get_random_number(int max_number);
WordStruct *get_first_random_word(LinkList *dictionary);
WordStruct *get_next_random_word(WordStruct *word_struct_ptr);
int generate_sentence(LinkList *dictionary);
void free_dictionary(LinkList *dictionary);

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int main(int argc, char *argv[])
{
    if(argc != 4 && argc != 5)//if user entered the wrong amount of arguments.
    {
        printf("usage: <seed> <tweets> <path> <num of words>");
        exit(EXIT_FAILURE);
    }
    int seed = atoi(argv[1]);
    srand(seed);
    int amountTweets = atoi(argv[2]);
    char* path = argv[3];
    FILE *fp;
    fp = fopen(path, "r");
    if(fp == NULL)//if user entered invalid path
    {
        printf("Error: failed opening file");
        exit(EXIT_FAILURE);
    }
    int amountWords = -1;//update amount of words to read to -1 in case the user didnt enter a number
    if(argc == 5)//if user entered the amount replace it
    {
        amountWords = atoi(argv[4]);
    }
    LinkList* dictionary;
    dictionary = (LinkList*)malloc(sizeof(LinkList));
    if(dictionary ==NULL)
    {
        printf("Allocating Failure: failed to locate memory");
        exit(EXIT_FAILURE);
    }
    dictionary->last=NULL;
    dictionary->first=NULL;
    dictionary->size=0;
    fill_dictionary(fp,amountWords,dictionary);//send to function to fill dictionary
    //print(dictionary);
    int i;
    for(i=0; i<amountTweets; i++)
    {
        printf("tweet %d: ",i+1);
        generate_sentence(dictionary);//send to function to get a tweet
        printf("\n");
    }
    free_dictionary(dictionary);//send to function to free all allocated variables
    fclose( fp );//close the file

    return 0;
}

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add(LinkList *link_list, WordStruct *data)
{
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node ==NULL)
    {
        printf("Allocating Failure: failed to locate memory");
        exit(EXIT_FAILURE);
    }

    *new_node = (Node){data, NULL};

    if (link_list->first == NULL)
    {
        link_list->first = new_node;
        link_list->last = new_node;
    }
    else
    {
        link_list->last->next = new_node;
        link_list->last = new_node;
    }

    link_list->size++;

    return 0;
}
/*************/

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number)
{
    int ranNumber =  (rand()%(max_number));//get random number between 0 and the given number
    return ranNumber;
}

/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct *get_first_random_word(LinkList *dictionary)
{
    do
    {
        int RandomNum = get_random_number(dictionary->size);//send the amount of words in dictionary to randum function
        Node *current = dictionary->first;
        while(current != NULL && RandomNum >0)//go over the dictionary and stop when reached the random number
        {
            RandomNum--;
            current = current->next;
        }
        if(current->data->lengthOfProbList != 0)//if the word is not and end of a sentence use it otherwise continue looking
        {
            return current->data;
        }
    }while(1);
}

/**
 * Choose randomly the next word. Depend on it's occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct *get_next_random_word(WordStruct *word_struct_ptr)
{
    do
    {
        int i;
        int randomNum= get_random_number(word_struct_ptr->occurrence);//send amount of times the word appears in file
        for(i=0; i<word_struct_ptr->lengthOfProbList;i++)
        {
            if(word_struct_ptr->prob_list[i].occurrenceInProbList>=randomNum)//if first word in prob list appears more times than the number that was given randomly
                return word_struct_ptr->prob_list[i].word_struct_ptr;//take it otherwise continue to next word
            randomNum = randomNum-word_struct_ptr->prob_list[i].occurrenceInProbList;//reduce the random number
        }

    }while(1);
}

/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence(LinkList *dictionary)
{
    int WordsInSentence = 1;
    int flag=0;
    WordStruct* firstWord = get_first_random_word(dictionary);
    WordStruct* nextWord;
    printf("%s ",firstWord->word);
    while(WordsInSentence < MAX_WORDS_IN_SENTENCE_GENERATION && flag == 0)//continue the sentence till get in random function end of sentence or sentence includes 20 words
    {
        if(WordsInSentence == 1)
            nextWord = get_next_random_word(firstWord);//send to function to get first random word of sentence
        else
            nextWord = get_next_random_word(nextWord);//send to function to get next random word

        WordsInSentence++;
        printf("%s ",nextWord->word);//print the word
        if(nextWord->lengthOfProbList==0)//if got a end word finish the sentence
        {
            flag = 1;
        }
    }
    return WordsInSentence;
}

/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list(WordStruct *first_word,WordStruct *second_word)
{
    if(first_word->word == NULL)
    {
        return 1;
    }

    if(first_word->word[strlen(first_word->word)-1] == '.')//if the word is an end word exit function
    {
        return 1;
    }

    if(first_word->lengthOfProbList == 0)//if word doesn't exist in dictionary
    {
        first_word->prob_list = (WordProbability *)malloc(sizeof(WordProbability));//locate place for prob list of the word
        if(first_word->prob_list ==NULL)
        {
            printf("Allocating Failure: failed to locate memory");
            exit(EXIT_FAILURE);
        }

        first_word->prob_list[0].word_struct_ptr = second_word;//put in first place of array the second word
        first_word->lengthOfProbList = 1;//update length of the prob list to be 1
        first_word->prob_list[0].occurrenceInProbList = 1;//update the times the second word appears after the first word to one
        return 1;
    }
    else//if word exists in the dictionary
    {
        int i;//check if exists in probability list
        for(i=0; i<first_word->lengthOfProbList; i++)
        {
            if(strcmp(first_word->prob_list[i].word_struct_ptr->word , second_word->word)==0)//if word exists in probability list
            {
                first_word->prob_list[i].occurrenceInProbList+=1;//update occurrence
                return 0;
            }
        }
        first_word->prob_list= (WordProbability *)realloc(first_word->prob_list,(first_word->lengthOfProbList+1)*sizeof (WordProbability));//if word doesn't exist in the prob list realloc the array to an extra place
        if (first_word->prob_list == NULL)
        {
            printf("Memory not allocated.\n");
            exit(EXIT_FAILURE);
        }

        first_word->prob_list[first_word->lengthOfProbList].word_struct_ptr = second_word;//put in the new place the second word
        first_word->prob_list[first_word->lengthOfProbList].occurrenceInProbList=1;//update  the amount of times it appears after the first word to 1
        first_word->lengthOfProbList += 1;//update the length of array to plus 1
    }
    return 0;
}
/**function that return the word in the diction
 * if word exists-> add the occurrence of the word and return the pointer to it
 * if the word doesn't exist the function will add the word to end of the dictionary and return the pointer to the last node in dictionary
 *as well the function will update the relevant parameters of the new word
 **/
WordStruct* getOrAddNode(LinkList *dictionary, char *word)
{
    WordStruct *newWord;
    Node *current = dictionary->first;
    while (current != NULL)
    {
        if (strcmp(current->data->word, word) == 0)//if word exists
        {
            current->data->occurrence += 1;//update occurrence
            free(word);//if word was found free the temp word that saved it
            return current->data;
        }
        current = current->next;
    }
    newWord =(WordStruct *)malloc(sizeof (WordStruct));//locate place for new node
    if(newWord ==NULL)
    {
        printf("Allocating Failure: failed to locate memory");
        exit(EXIT_FAILURE);
    }
    newWord->word=word;//update word to be the word
    newWord->prob_list = NULL;//prob list of it is empty
    newWord->lengthOfProbList=0;//length is 0

    add(dictionary,newWord);//send to function to add to dictionary
    dictionary->last->data->occurrence=1;
    return dictionary->last->data;//return relevant pointer
}


/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary(FILE *fp, int words_to_read, LinkList *dictionary)
{

    char line[MAX_SENTENCE_LENGTH];
    char* p;
    char* p2;
    WordStruct* getNode;
    WordStruct* prevWord = NULL;

    while(fgets(line,MAX_SENTENCE_LENGTH,fp) != NULL)//continue reading words till the end of the file
    {
        p = strtok(line," \n");//first time update the first word to end in space or \n
//        if(p[strlen(p)-1]=='\n')//if last character in word is enter
//            p[strlen(p)-1]='\0';//replace the enter character with the end of word
        prevWord = NULL;
        while(p != NULL)
        {
            if(words_to_read ==0)//if finished reading the amount of words given
            {
                return;
            }
            --words_to_read;

            p2 = (char*)malloc((sizeof (char)*(strlen(p)+1)));//copy the strtok token to new chat
            if(p2 ==NULL)
            {
                printf("Allocating Failure: failed to locate memory");
                exit(EXIT_FAILURE);
            }
            strcpy(p2,p);//copy the word
            getNode = getOrAddNode(dictionary,p2);//send to function to add or get the node
            if(prevWord!=NULL)
                add_word_to_probability_list(prevWord,getNode);//send to function to add second word to first word probability array
            prevWord=getNode;//copy first word to second word
            p = strtok(NULL," \n");//promote to thw second word
        }
    }

}

/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList *dictionary)
{
    Node * current = dictionary->first;

    while(current != NULL)//run on all dictionary
    {
        free(current->data->word);//free the word
        free(current->data->prob_list);//free the array of probability
        free(current->data);//free the Word struct
        dictionary->first = dictionary->first->next;//make next node to be first node of dictionary
        free(current);//free the node
        current=dictionary->first;
    }
    free(dictionary);//free the dictionary
}
