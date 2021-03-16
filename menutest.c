/*	Program:	menu.c
	Date:		10/25/94
			Updated 10/23/2001 GMA adding configurable 
			site name and menu home

	Programmer:	Mark Ayers
	Function:	Menu for running reports/processes 

		Input:	User-menu choices   
			File-menu.dat  menu choices, help line, helpfile, & command line
					for reports.  menu.dat is a text file
					each item separated by a colon ";"
				File format:
					Item Name (20)
					Command	  (40)
					Help      (75)
					Group ??  (15) 
						   (Novell-like permission
						    Item does not display
						    if user not in group)
					Help Text File Name (75)

			
			File-group.dat
					User ID and list of groups they
					belong to.  If user is not in
					group file exit with error.
					This will force security on reports.
					Record format userid:group1,group2,etc
					
				File format:
					User ID
					groups	

			File-header.dat 
					School Name & file locations

				File format:
					Item  (80) (school name)
					menu_home (120) (directory Name)						
		Output: Screen menu from menu.dat
			Shell script/report
			Help Screen Displays Comment portion of Report

		program -h returns help screen with this information


**************** 05/22/02 *********** ******************************
Mark Ayers	Added "Next Page" message to screen
		 Maybe add "^" for page up and "V" for page down.
********************************************************************

*/

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#define  MAX_ARRAY_SIZE		250	/*  largest acceptable array 	*/
#define  MENU_TEXT_SIZE		20 	/*  Menu text string size    	*/
#define  MENU_COMMAND_SIZE 	40 	/*  Menu command size		*/
#define  MENU_HELP_SIZE		75	/*  Item Help max size		*/
#define  MENU_GROUP_SIZE	15	/*  Group size			*/
#define  MENU_HELP_COMMAND	80	/*  Help Command size			*/
#define  FILE_NAME		"/u02/app/Dtae/Menu/menu.dat"   /*  menu data file		*/
#define  HEADER_FILE_NAME	"/u02/app/Dtae/Menu/header.dat"   /*  menu header file		*/
#define  GROUP_FILE_NAME	"/u02/app/Dtae/Menu/group.dat"	/* groups & users	*/
#define  GROUP_ARRAY_SIZE	30	/*  Max number of Groups a user
					    can belong to		*/
#define  MIN_LINES		20	/*  Min required terminal lines 	*/
#define  MIN_COLS		30	/*  Min required terminal columns 	*/
#define  WIN_COL_START		3
#define  WIN_ROW_START		5
#define  WIN_TEXT_COL		2
#define  WIN_HELP_COL		40
#define  WIN_WIDTH		70
#define  WIN_HEIGHT		12
#define  HELP_WIN_COL_START	3	/* Help window start column	*/
#define  HELP_WIN_ROW_START	5	/* Help window start row	*/
#define  HELP_WIN_WIDTH		70	/* Help window width		*/
#define  HELP_WIN_HEIGHT	12	/* Help window height		*/
#define  MAX_DISP_ARRAY		15	/* Max number of rows to display*/
#define  SCR_START		4	/* Row to start item display  	*/
#define  SCR_END		18	/* Row to end item display	*/
#define  VERSION		"3.0"
 
struct {
	char text[MENU_TEXT_SIZE];
	char comm[MENU_COMMAND_SIZE];
	char help[MENU_HELP_SIZE];
	char group[MENU_GROUP_SIZE];
	char help_command[MENU_HELP_COMMAND];
} menu_array[MAX_ARRAY_SIZE];


char group_array[GROUP_ARRAY_SIZE][MENU_GROUP_SIZE];
int group_array_size=0;
int array_size=0;
char site_name[90];
char home[90];
char userid[9];

int read_array(void);
int disp_help(int);
int disp_menu(int);
int load_groups(void);
int center_string(char *,int);
int val_group(int );	
int debug_it(void);
int load_header(void);
int help_screen(void);


int main(int argc,char *argv[])
	{
	int ans,element=0,a=0;
	int next_page=0;	/* 0 if no next page, 1 if there is a next page */
	int prev_page=0;	/* 0 if at first record 1 if element array > 1  */
	char str[80];
	if (argc > 1 && strcmp(argv[1],"H") ) { help_screen(); }
	strcpy(userid,getlogin()); 
	load_header();		/*  load heading*/
	load_groups();		/*  determine user groups		*/
	read_array(); 		/*  opens menu.dat file and loads array */	
	initscr();		/*  inits curses	*/
	raw();
	ans=0;
	do {
		switch(ans) {
			case 'T':  	/*  goto top of list */
			   	element=0;
			 	break;
			case 'B':	/* goto Bottom of List	*/
			   	element=array_size-MAX_DISP_ARRAY;
		 	   	break;
			case 'P':
				element-=MAX_DISP_ARRAY;
			   	break;
			case 'N':
				element+=MAX_DISP_ARRAY;
			   	break;
			case 'H':
			   	break;
			case '0':
			   	break;
			case 'D':
				debug_it();
				break;
			}
		if ( element >=array_size ) 
			element = array_size - MAX_DISP_ARRAY;
		if ( element < 0 ) element=0;
			
		if (ans > 99 ){ 
			disp_help(ans);
		}
		if (ans > 0 && ans <= array_size+1) {
			clear();
			refresh();
			endwin();
			clear();
   			system(menu_array[ans-1].comm);                   
			printf("End of %s program, press <ENTER> to return to menu: ",
				menu_array[ans-1].comm);
			gets(str);
			initscr();
			}
		clear();
		refresh();
	}while ((ans=disp_menu(element)) != '0');
	erase();
	refresh();
	endwin();		/*  closes curses window		*/
	puts("End of DTAE menu program");
}

int disp_menu(int element){	

			/* Displays array and returns choice    
			 N (90) moves screen down 1 pg 
			 P (91) moves screen up 1 pg
			 T (92) top of list
			 B (93) Bottom of list
			 number + H(h) displays help text Number+100

			Element is the starting array element to display.
			Element is validated in the MAIN procedure
			*/

	int ans,tmpans;
	int ctr;
	char str[180],tmpstr[180];
	char next_char, prev_char;

/*	if( element == 0 ) 
		next_char=' '; 
		else 
		next_char='^';
		

	if( element+MAX_DISP_ARRAY < array_size ) 
		prev_char='V'; 
		else  
		prev_char=' ';
*/		

	WINDOW *menu_win;
	box(stdscr,'|','-');
	center_string(site_name,1);	
	move(1,2);
	printw("%s",VERSION);
	move(1,70);
	/* printw("%s %s %s %s",tm_wday,tm_mon,tm_year); */
	move(80,2);
	center_string("Report /Process Menu",2);		
	center_string("N-Next Page   P-Previous Page   T-Top of List   B-Bottom of List",22);
	center_string("Enter the Item number and an 'H' for additional Help",23); 
	center_string("Enter Choice (0 to quit):     ",20);
	move(1,20);
	printw("%s - %s",next_char, prev_char);
		
	/*  draw array	*/	
	for ( ctr = SCR_START;ctr <= SCR_END;ctr++ ){
		move(ctr,13);
		printw("%2d- %-15s   %.75s",element+1,
			menu_array[element].text,
			menu_array[element++].help);
		if (element >= array_size) break;
		}

	move(20,(COLS/2)+10);
	getstr(str);
	switch ( str[0] ) { 
		case 'd':  /* debug */
		case 'h':
		case 't': 
		case 'n':
		case 'p': 
		case 'b': str[0]=toupper(str[0]); 
		case 'D': /* debug */
		case 'T': 
		case 'N': 
		case 'P': 
		case 'B':
		case '0':
			return str[0];
		}
	
	if ( toupper(str[1]) == 'H' || toupper(str[2]) == 'H' ||toupper(str[3]) == 'H' ) {
		ans=atoi(str)+100;
		return ans;
	}
	ans=atoi(str);

	if( ans < 1 || ans > MAX_ARRAY_SIZE )
		return 0;
	else	
		return ans;
	
}


/*	disp_help()  displays the help line from the array at the
	bottom of the screen.  The argument ctr is the element
	in the array to be displayed
*/
int disp_help(int numb){
	char str[180];
	erase();
	refresh();
	move(1,2);
/*	printw("\npre calc counter = %i\n",numb);  */
	numb = numb-100;
	if (numb > array_size)  return; 
/*	printw("\n%s\n",menu_array[numb].help); 
	printw("%s\n",menu_array[numb].help_command); 
	printw("Counter = %i\n",numb); 
	printw("Help has been called\n ");  */
 	clear();
        refresh(); 
        endwin();
	system("clear");
	system("cat viewer_screen_help.txt");
	printf("Press <ENTER> to view %s help/documentation screen: ",menu_array[numb-1].text);
	gets(str);
	sprintf(str,"pg %s\n",menu_array[numb-1].help_command);
  	system(str);
	initscr();
	refresh();
	return;
}


int read_array(){	 /*  opens menu.dat file and loads array */
	FILE *fp;
	int c;
	int group_sw=0;
	int flag=0;
	int ctr=0;
	int group_ctr=0;
	int a,b,tmp_c,tmp_r;
	char tmp[160];

	if ( (fp = fopen(FILE_NAME,"r"))== NULL){
		printf("\nmenu: cannot open file %s\n",FILE_NAME);
		exit(-1);
	}
	while ((c = getc(fp)) != EOF ){
		switch ( c ){
			case ';' : 
						/* set end of string   	*/
				if (flag == 0 )
				    menu_array[array_size].text[ctr++]='\0';
				if (flag == 1 ) 
				    menu_array[array_size].comm[ctr++]='\0';
				if (flag == 2 ) 
				    menu_array[array_size].help[ctr++]='\0';
				if (flag == 3 ) 
				    menu_array[array_size].group[ctr++]='\0';
				if (flag == 4 ) 
				    menu_array[array_size].help_command[ctr++]='\0';
				flag++;  /*  field delimiter   */
				ctr=0;
				continue;
				break;
			case '\n': 
						/* set end of string   	*/
				if (flag == 0 )
				    menu_array[array_size].text[ctr++]='\0';
				if (flag == 1 ) 
				    menu_array[array_size].comm[ctr++]='\0';
				if (flag == 2 ) 
				    menu_array[array_size].help[ctr++]='\0';
				if (flag == 3 ) 
				    menu_array[array_size].group[ctr++]='\0';
				if (flag == 4 ) 
				    menu_array[array_size].help_command[ctr++]='\0';
				/*  check for group	*/
				if ( val_group(array_size)) 
					array_size++;  /* record delimiter  */ 
				ctr=0;
				flag=0;
				continue;
				break;
		}
		if (flag == 0 ) menu_array[array_size].text[ctr++]=c;
		if (flag == 1 ) menu_array[array_size].comm[ctr++]=c;
		if (flag == 2 ) menu_array[array_size].help[ctr++]=c;
		if (flag == 3 ) menu_array[array_size].group[ctr++]=c;
		if (flag == 4 ) menu_array[array_size].help_command[ctr++]=c;
	}
	
	fclose(fp); 

/*      To get rid of -1 array size   
	array_size -= 1;
*/
	if ( array_size > MAX_ARRAY_SIZE ) {
		printf("Error menu: File %s exceeds %i record limit... ",
			FILE_NAME,
			MAX_ARRAY_SIZE);
		exit(-1);
	}	

	/*List all records for debugging  */

	for (ctr=0;ctr< array_size;ctr++){
	printf("menu_array[%i].text = %s\n",ctr,menu_array[ctr].text);
	printf("menu_array[%i].comm = %s\n",ctr,menu_array[ctr].comm);
	printf("menu_array[%i].help = %s\n",ctr,menu_array[ctr].help);
	printf("menu_array[%i].group = %s\n",ctr,menu_array[ctr].group);
	printf("menu_array[%i].help_command = %s\n",ctr,menu_array[ctr].help_command);
	}
	gets;
/*	*/
}


int load_groups(void){    	/* Loads groups from groups file	*/
	
	FILE *fp;
	char str[160],tempstr[160];
	int ctr=0,ctr2=0,sw=0;

	if ( (fp = fopen(GROUP_FILE_NAME,"r"))== NULL){
		printf("\nMenu: cannot open file %s\n",GROUP_FILE_NAME);
		exit(-1);
	}

	while ( fgets(str,160,fp) != NULL ){
		ctr=0;				/* reset char ctr	*/
		while ( str[ctr] != ':' ){
			tempstr[ctr] = str[ctr++];
		}				/* get userid		*/
		tempstr[ctr]='\0';
		if ( strcmp(tempstr,userid) == 0 ) {
			sw=1;
			break;  /* found user	*/

		}
		
	}
	if ( !sw  ){  
		printf("\nError menu: User not found\n");
		exit(-1);
		}

	ctr2=0;
	sw=0;
	
	for( ctr=0;ctr<strlen(str);ctr++){
		if ( str[ctr] == ':' ){
			sw=1;		/* found end of user ID		*/
			continue;
			}	
		if ( !sw ){
			continue; 	/* continue for until find :	*/ 	
			}
		if (str[ctr] == ',' ){
			group_array[group_array_size++][ctr2]='\0';
					/* fill array with blanks	*/
			for (;ctr2<=MENU_GROUP_SIZE;ctr2++) {
				group_array[group_array_size][ctr2]=' ';
				}
			ctr2=0;
			continue;
			}
		if ( str[ctr] != '\n' ) 
		group_array[group_array_size][ctr2++]= str[ctr];
		}
	group_array[group_array_size][ctr2]='\0';
		
	fclose(fp);
}

/*	Load Header Data		*/

int load_header(void){    	/* Loads groups from groups file	*/
	
	FILE *fp;
	char str[160],tempstr[160];
	int ctr=0,ctr2=0,sw=0;
	

	if ( (fp = fopen(HEADER_FILE_NAME,"r"))== NULL){
		printf("\nMenu: cannot open file %s\n",HEADER_FILE_NAME);
		exit(-1);
	}

	while ( fgets(str,160,fp) != NULL ){
		ctr=0;				/* reset char ctr	*/
		while ( str[ctr] != ':' ){
			tempstr[ctr] = str[ctr++];
		}				/* get Header		*/
		tempstr[ctr]='\0';
		if ( strcmp(tempstr,"Item") == 0 ) {
			ctr++;
			sw=1;
			ctr2=0;
       			while ( str[ctr] != '\n' ){	
        		        tempstr[ctr2++] = str[ctr++];
       				}
      			strcpy(site_name,tempstr);
		        site_name[ctr2]='\0';
			/* break;  loaded site name 	*/
		}	
		if ( strcmp(tempstr,"home") == 0 ) {
			ctr++;
			sw=2;
			ctr2=0;
       			while ( str[ctr] != '\n' ){	
        		        tempstr[ctr2++] = str[ctr++];
       				}
      			strcpy(home,tempstr);
		        home[ctr2]='\0';
			/*	break;   loaded site home 	*/
		}	
	}
	
	if ( !sw  ){  
		printf("\nError menu: Site Name not found\n");
		exit(-1);
		}
		
	if ( sw==1  ){  
		printf("\nError menu: Menu Home not found\n");
		exit(-1);
		}
	fclose(fp);
}		/*	Load Header	*/

int center_string(char *str,int line) {
	int a;
	a=((COLS/2)-(strlen(str)/2));
	move(line,a);
	printw("%s",str);

}

int val_group(int array_element){	
		/* return 1 if element in group otherwise 0  	*/

	int group_ctr;
	int group_sw=0;

	for (group_ctr =0; group_ctr<=group_array_size;group_ctr++){
		if ( strcmp(menu_array[array_element].group,
			group_array[group_ctr]) == 0 ){
			group_sw=1;	
			}
		}
	return (group_sw );	
}


int debug_it(void){
	char str[81];
	int c;

	do {
	clear();
	refresh();
	printf("\n%s\n%s\n%s\n%s\n%s",
		"<L>ist Array",
		"<U>ser ID",
		"<G>roup(s)",
		"<V>ariables",
		"<Q>uit");
	printf("\nEnter choice");
	gets(str);
	switch(str[0]){
		case 'L':
		case 'l':
			for( c=0;c<=array_size;c++){
			printf("\n\nText:%s \nComm:%s \nHelp:%s \nGroup:%s",
				menu_array[c].text,
				menu_array[c].comm,
				menu_array[c].help,
				menu_array[c].group);
			}
			printf("\n\n");
			break;
		case 'U':
		case 'u':
			printf("\nUser: %s",userid);
			break;
		case 'G':
		case 'g':
			printf("\nGroups");
			for( c=0;c<=group_array_size;c++)
			printf("\n Group:%s", group_array[c]);
			break;
		case 'V':
		case 'v':
			printf("\nArray Size %i \nGroup Size %i ",
				array_size,
				group_array_size);
			break;
		case 'Q':
		case 'q':
			return;
	}
	printf("\nPress <ENTER> to continue...");
	gets(str);

	}
	while(1);
}

int help_screen(void){
	system("head -49 menu.c|pg");
	
	exit(-1);

}
