/* ---------------------------------------------- *
 * baht 
 * =========
 * 
 * Summary
 * -------
 * A robot scraper thingy, which helps build the database 
 * for CheapWhips.net...
 * 
 * Usage
 * -----
 * tbd
 * 
 * Author
 * ------
 * Antonio R. Collins II (ramar@tubularmodular.com, ramar.collins@gmail.com)
 * Copyright: Tubular Modular Feb 9th, 2019
 * 
 * TODO
 * ----
 - email when done
 - stream to db
 - add options 
 - test out hashes against the table with other types of markup
 - add a way to jump to a specific hash and do the search from there (easiest
   way is to copy the table)
	 Table *nt = lt_copybetween( Table tt, int start, int end );  
	 free( tt );
 - add the option to read directly from memory (may save time)
 - add the option to read from hashes from text file (better than recompiling if
   something goes wrong)
 * ---------------------------------------------- */
#if 0 
#include <stdio.h> 
#include <unistd.h> 
#include <errno.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include <string.h> 
#endif
#include "vendor/single.h"
#include <gumbo.h>
#define PROG "p"


typedef struct nodeset {
	int hash;             //Stored hash
	const char *key;      //Key that the value corresponds to
	const char *string;   //String
} NodeSet;

typedef struct nodeblock {

	//The content to digest
	//const uint8_t *html;

	//The "root element" that encompasses the elements we want to loop through
	NodeSet rootNode;

	//A possible node to jump to
	NodeSet jumpNode;

	//A set of elements containing entries
	NodeSet *loopNodes;

	//Set of tables (each of the nodes, copied)
	Table *tlist;

} Nodeblock;


Nodeblock nodes[] = {
/*.content = "files/carri.html"*/
	{ 
		.rootNode = {.string = "div^backdrop.div^content_a.div^content_b.center"} 
	 //,.jumpNode = { .string = "div^backdrop.div^content_a.div^content_b.center" } 
	 ,.jumpNode = {.string = "div^backdrop.div^content_a.div^content_b.center.div^thumb_div" }

	//,.deathNode = {.string= "div^backdrop.div^content_a.div^content_b.center.br" }
	//,.miniDeathNode = {.string="div^backdrop.div^content_a.div^content_b.center.input#raw_price_13" }
	}
};



typedef struct useless_structure {
	LiteKv *parent;
	Table *srctable;
	char *key;
	int jump;
	int keylen;
	int hlistLen;
	int tlistLen;
	int *hlist;
	Table *tlist;
} InnerProc;



//Things we'll use
static int gi=0;
const char *gumbo_types[] = {
	"document"
, "element"
, "text"
, "cdata"
, "comment"
, "whitespace"
, "template"
};

//Test data
const char *html_1 = "<h1>Hello, World!</h1>";
const char html_file[] = "files/carri.html";
const char yamama[] = ""
"<html>"
	"<body>"
		"<h2>Snazzy</h2>"
		"<p>The jakes is coming</p>"
		"<div>"
			"<p>The jakes are still chasing me</p>"
			"<div>"
				"<p>Son, why these dudes still chasing me?</p>"
			"</div>"
		"</div>"
	"</body>"
"</html>"
;


//Return the type name of a node
const char *print_gumbo_type ( GumboNodeType t ) {
	return gumbo_types[ t ];
}


//Find a specific tag within a nodeset 
GumboNode* find_tag ( GumboNode *node, GumboTag t ) {
	GumboVector *children = &node->v.element.children;

	for ( int i=0; i<children->length; i++ ) {
		//Get data "endpoints"
		GumboNode *gn = children->data[ i ] ;
		const char *gtagname = gumbo_normalized_tagname( gn->v.element.tag );
		const char *gtype = (char *)print_gumbo_type( gn->type );

		//I need to move through the body only
		if ( gn->v.element.tag == t ) {
			return gn;
		}
	}
	return NULL;
}


//Return appropriate block
char *retblock ( GumboNode *node ) {

	char *iname = NULL;
	//Give me some food for thought on what to do
	if ( node->type == GUMBO_NODE_DOCUMENT )
		iname = "d";
	else if ( node->type == GUMBO_NODE_CDATA )
		iname = "a";
	else if ( node->type == GUMBO_NODE_COMMENT )
		iname = "c";
	else if ( node->type == GUMBO_NODE_WHITESPACE )
		iname = "w";
	else if ( node->type == GUMBO_NODE_TEMPLATE )
		iname = "t";
	else if ( node->type == GUMBO_NODE_TEXT )
		iname = (char *)node->v.text.text;
	else if ( node->type == GUMBO_NODE_ELEMENT ) {
		iname = (char *)gumbo_normalized_tagname( node->v.element.tag );
	}

	return iname;
}


#if 0
//Gumbo to Table 
int lua_to_table (lua_State *L, int index, Table *t ) {
	static int sd;
	lua_pushnil( L );
	obprintf( stderr, "Current stack depth: %d\n", sd++ );

	while ( lua_next( L, index ) != 0 ) 
	{
		int kt, vt;
		obprintf( stderr, "key, value: " );

		//This should pop both keys...
		obprintf( stderr, "%s, %s\n", lua_typename( L, lua_type(L, -2 )), lua_typename( L, lua_type(L, -1 )));

		//Keys
		if (( kt = lua_type( L, -2 )) == LUA_TNUMBER )
			obprintf( stderr, "key: %lld\n", (long long)lua_tointeger( L, -2 ));
		else if ( kt  == LUA_TSTRING )
			obprintf( stderr, "key: %s\n", lua_tostring( L, -2 ));

		//Values
		if (( vt = lua_type( L, -1 )) == LUA_TNUMBER )
			obprintf( stderr, "val: %lld\n", (long long)lua_tointeger( L, -1 ));
		else if ( vt  == LUA_TSTRING )
			obprintf( stderr, "val: %s\n", lua_tostring( L, -1 ));

		//Get key (remember Lua indices always start at 1.  Hence the minus.
		if (( kt = lua_type( L, -2 )) == LUA_TNUMBER )
			lt_addintkey( t, lua_tointeger( L, -2 ) - 1);
		else if ( kt  == LUA_TSTRING )
			lt_addtextkey( t, (char *)lua_tostring( L, -2 ));

		//Get value
		if (( vt = lua_type( L, -1 )) == LUA_TNUMBER )
			lt_addintvalue( t, lua_tointeger( L, -1 ));
		else if ( vt  == LUA_TSTRING )
			lt_addtextvalue( t, (char *)lua_tostring( L, -1 ));
		else if ( vt == LUA_TTABLE )
		{
			lt_descend( t );
			obprintf( stderr, "Descending because value at %d is table...\n", -1 );
			lua_loop( L );
			lua_to_table( L, index + 2, t ); 
			lt_ascend( t );
			sd--;
		}

		obprintf( stderr, "popping last two values...\n" );
		if ( vt == LUA_TNUMBER || vt == LUA_TSTRING ) {
			lt_finalize( t );
		}
		lua_pop(L, 1);
	}

	lt_lock( t );
	return 1;
}
#endif



Table *tt;
int twoSided = 0;

//Go through and run something on a node and ALL of its descendants
//Returns number of elements found (that match a certain type)
//TODO: Add element count 
//TODO: Add filter for element count
int rr ( GumboNode *node ) {
	//Loop through the body and create a "Table" 
	GumboVector *bc = &node->v.element.children;
	int stat = 0;

	//For first run, comments, cdata, document and template nodes do not matter
	for ( int i=0; i<bc->length; i++ ) {
		//Set up data
		GumboNode *n = bc->data[ i ] ;
		char *itemname = retblock( n );

#if 0
		//Dump data if needed
		//TODO: Put some kind of debug flag on this
		char *type = (char *)print_gumbo_type( n->type );
		fprintf( stderr, "%06d, %04d, %-10s, %s\n", ++gi, i, type, itemname );
#endif

#if 1
		//Handle what to do with the actual node
		//TODO: Handle GUMBO_NODE_[CDATA,COMMENT,DOCUMENT,WHITESPACE,TEMPLATE]
		if ( n->type != GUMBO_NODE_TEXT && n->type != GUMBO_NODE_ELEMENT ) {
			//User selected handling can take place here, usually a blank will do
			twoSided = 0; 
			//lt_addnullvalue( t, itemname );
		}
		else if ( n->type == GUMBO_NODE_TEXT ) {
			//Clone the node text as a crude solution
			int cl=0;
			unsigned char *mm = trim( (unsigned char *)itemname, " \t\r\n", strlen(itemname), &cl );
			char *buf = malloc( cl + 1 );
			memset( buf, 0, cl + 1 );
			memcpy( buf, mm, cl );	
			//Handle/save the text reference here
			lt_addtextkey( tt, "text" );
			lt_addtextvalue( tt, buf ); 
			lt_finalize( tt );
			free( buf );	
		}
		else if ( n->type == GUMBO_NODE_ELEMENT ) {
			GumboVector *gv = &n->v.element.children;
			GumboVector *gattr = &n->v.element.attributes;
			GumboAttribute *attr = NULL;
			char item_cname[ 2048 ];
			int maxlen = sizeof( item_cname ) - 1;
			memset( item_cname, 0, maxlen ); 
			char *iname = itemname;

			//This newest change will add an Id or class to a hash
			if ( gattr->length ) {
				if ( ( attr = gumbo_get_attribute( gattr, "id" ) ) ) {
					//fprintf( stderr, "id is: #%s\n", attr->value );	
					snprintf( item_cname, maxlen, "%s#%s", itemname, attr->value ); 	
					iname = item_cname;
				}

				if ( ( attr = gumbo_get_attribute( gattr, "class" ) ) ) {
					//fprintf( stderr, "class is: .%s\n", attr->value );	
					snprintf( item_cname, maxlen, "%s^%s", itemname, attr->value ); 	
					iname = item_cname;
				}
			}

			//Should always add this first
			lt_addtextkey( tt, iname );
			lt_descend( tt );
	
			//node elements should all have attributes, I need a list of them, then
			//need them written out
			if ( gattr->length ) {
				lt_addtextkey( tt, "attrs" );
				lt_descend( tt );
				for ( int i=0; i < gattr->length; i++ ) {
					GumboAttribute *ga = gattr->data[ i ];
					lt_addtextkey( tt, ga->name );
					lt_addtextvalue( tt, ga->value );
					lt_finalize( tt );
				}
				lt_ascend( tt );
			}

#if 0
			if ( !gv->length ) {
				lt_addtextkey( tt, "text" );	
				lt_addtextvalue( tt, "(nothing)" );	
				lt_finalize( tt );
			}
			else {
#endif
			if ( gv->length ) {
				rr( n );
			}

			lt_ascend( tt );
		}
#endif
	}

	lt_lock( tt );
	return 1;
}


//This is weird, and probably not efficient, but it will have to do for now.
int extract_same ( LiteKv *kv, int i, void *p ) {
	//The parent should be at a specific address and not change during each new invocation
	//The FULL key should also be the same (not like the child couldn't, but its less likely)
	//A data structure can take both of these...
	InnerProc *pi = (InnerProc *)p;

	//Super debugging function :)
	//fprintf( stderr, "@%5d: %p %c= %p\n", i, kv->parent, ( kv->parent == pi->parent ) ? '=' : '!', pi->parent );

	//Check that parents are the same... 
	if ( kv->parent && (kv->parent == pi->parent) && (i >= pi->jump) ) {
		//...and that the full hash matches the key.
		unsigned char fkBuf[ 2048 ] = {0};
		if ( !lt_get_full_key( pi->srctable, i, fkBuf, sizeof(fkBuf) - 1 ) ) {
			return 1;
		}

		//More debugging
		//fprintf( stderr, "fk: %s\n", (char *)lt_get_full_key( pi->srctable, i, fkBuf, sizeof(fkBuf) - 1 ) );
		//fprintf( stderr, "%d ? %ld\n", pi->keylen, strlen( (char *)fkBuf ) ); 

		//Check strings and see if they match? (this is kind of a crude check)
		if ( pi->keylen == strlen( (char *)fkBuf ) && memcmp( pi->key, fkBuf, pi->keylen ) == 0 ) {
			//save hash here and realloc a stretching int buffer...
			if ( pi->hlist ) 
				pi->hlist = realloc( pi->hlist, sizeof(int) * pi->hlistLen );
			else {
				pi->hlist = malloc( sizeof(int) );
				*pi->hlist = 0;
			}

			//???
			*(&pi->hlist[ pi->hlistLen ]) = i;
			pi->hlistLen++;	
		} 
	}

	return 1;
}


//
int build_individual ( LiteKv *kv, int i, void *p ) {
	//Deref and get the first table in the list.
	InnerProc *pi = (InnerProc *)p;
	Table *ct = pi->tlist;

	//Get type of value, save accordingly.
	//TEXT 
	//BLOB
	//INT
	//FLOAT
	//TABLE
#if 0
	lt_addtextkey( ct, kv->v.key.vchar );	
	lt_addtextvalue( ct, kv->v.value.vchar );	
	lt_finalize( ct );
#endif
	return 1;
}


//Much like moving through any other parser...
int main() {

	int fn, len;
	struct stat sb;
	char *block = NULL;

#if 0
	block = (char *)yamama;
	len = strlen( block );
#else
	char fb[ 2000000 ] = {0};

	//Read file to memory
	if ( stat( html_file, &sb ) == -1 ) {
		fprintf( stderr, "%s: %s\n", PROG, strerror( errno ) );
		return 1; 
	}

	if ( (fn = open( html_file, O_RDONLY )) == -1 ) {
		fprintf( stderr, "%s: %s\n", PROG, strerror( errno ) );
		return 1; 
	}
	
	if ( read( fn, fb, sb.st_size ) == -1 ) {
		fprintf( stderr, "%s: %s\n", PROG, strerror( errno ) );
		return 1; 
	}

	block = fb;
	len = sb.st_size;
#endif

	//Parse that and do something
	GumboOutput *output = gumbo_parse_with_options( &kGumboDefaultOptions, block, len ); 

	//We can loop through the Gumbo data structure and create a node list that way
	GumboVector *children = &(output->root)->v.element.children;
	GumboNode *body = find_tag( output->root, GUMBO_TAG_BODY );
	if ( !body ) {
		fprintf( stderr, PROG ": no <body> found!\n" );
		return 1;
	}

	//Allocate a Table
	Table t;
	tt = &t;
	if ( !lt_init( &t, NULL, 33300 ) ) {
		fprintf( stderr, PROG ": couldn't allocate table!\n" );
		return 1;
	}

	//Run the parser against everything
	//TODO: Call this something more clear than rr()
	int elements = rr( body );
	lt_dump( tt );

	//Loop through each of the requested nodes
	//This will probably look more like:
	//1. find "mini-root" or "loop" node
	//2. copy a table from "loop" node (or start node) to an end node
	//3. stream to database structure or whatever...
	for ( int i=0; i<sizeof(nodes)/sizeof(NodeSet); i++ ) {
#if 0	
		//1. Read each line (of YAML or whatever) into this structure	
		struct s { char *left, *right; } list = chop_line( *line );
		//1a. This could come from a compiled in data structure, but that's not the
		//best method here...
#endif

		//References
		unsigned char fkbuf[ 2048 ] = { 0 };
		NodeSet *root = &nodes[ i ].rootNode; 
		NodeSet *jump = &nodes[ i ].jumpNode; 
		//NodeSet *loop = &nodes[ i ].loopNodes; 

		//2. Second step is to simply find the node
		if ( ( root->hash = lt_geti( tt, root->string ) ) == -1 ) {
			fprintf( stderr, PROG ": string '%s' not found.\n", root->string );
			exit( 0 );
		}
		fprintf(stderr, "@%-5d -> %s\n", root->hash, root->string );

		//2A. Jumping basically means that in the root node, there are many elements which we just don't need to worry about.	
		//In this particular case, I need to find the first thing that matches the jump node, and/or input this myself.
		//Let's try matching the jump node.
		if ( ( jump->hash = lt_geti( tt, jump->string ) ) == -1 ) {
			fprintf( stderr, PROG ": string '%s' not found.\n", jump->string );
			exit( 0 );
		}
		fprintf(stderr, "@%-5d -> %s\n", jump->hash, jump->string );

		//Get parent and do work.
		char *fkey = (char *)lt_get_full_key( tt, jump->hash, fkbuf, sizeof(fkbuf) - 1 );
		InnerProc pp = {
			.parent   = lt_retkv( tt, root->hash )
		 ,.srctable = tt
		 ,.jump = jump->hash 
		 ,.key  = fkey 
		 ,.keylen = strlen( fkey )
		 ,.tlist = NULL
		 ,.tlistLen = 0
		 ,.hlist = NULL
		 ,.hlistLen = 0
		};

		//Start the extraction process 
		lt_exec( tt, &pp, extract_same );
		lt_reset( tt );

		//Looping and being weird
		for ( int i=0; i<pp.hlistLen; i++ ) {
			//fprintf( stderr, "hash: %d\n", pp.hlist[ i ] );
			//rebuild with this function
			if ( !lt_absset( tt, pp.hlist[ i ] ) ) {
				fprintf( stderr, "Couldn't reset table pointer.\n" );
				exit( 0 );
			}

			//How do I see the table?
			LiteKv *ct = lt_current( tt );
			fprintf( stderr, "%d -> %p\n", pp.hlist[ i ], ct );
#if 1
			//TODO: For our purposes, 5743 is the final node.  Fix this.
			int start, end;
			start = pp.hlist[ i ];
			end = ( i+1 > pp.hlistLen ) ? tt->count : pp.hlist[ i+1 ]; 

#if 0
			//Add space for new table here
			if ( pi->tlist ) 
				pi->tlist = realloc( pi->hlist, sizeof(Table) * pi->hlistLen );
			else {
				pi->tlist = malloc( sizeof(Table) );
				*pi->tlist = 0;
			}

			//???
			*(&pi->hlist[ pi->hlistLen ]) = i;
			pi->hlistLen++;	
#endif

			//Check the status, cuz it could be false...
			lt_exec_complex( tt, start, end, &pp, build_individual );
#endif
		}

		//After building the miniatures, it's totally feasible to destroy the original table.
		//lt_destroy( tt );

		//You should also test the miniatures
		//for ( int ti=0; ti < tablelist.len ; ti++ ) {
		//	Table *mt = &(pp.tlist)[ ti ];
		//	lt_dump( mt );
		//}

		//Then for each new table, find all the hashes
#if 0
		//Hold all of the database records somewhere.
		DBRecord *records = NULL 

		//Loop through each table in the set
		for ( int ti=0; ti < tablelist.len ; ti++ ) {

			//Mark the node
			Table *mt = &(pp.tlist)[ ti ];
			
			//Using a list of strings from earlier, find each node in the NEW table
			for ( int ii = 0; ii < stringsToHash.len; ii++ ) { 

				//Get the node ref
				char *res = ( char * )find_node( mt, stringsToHash[ ii ] );

				//Save the record
				if ( !res ) 
					;
				else {
					*record = { SQL_TYPE, 'column', res };
					record++;
				}

			}

			//It might save memory to go through all the records after each run and reduce memory usage.
			//db_exec( &record... ) ;  //clearly have no idea how this works...
			//free( record );
			//destroy mini table
			//lt_free( mt );
#endif
			exit( 0 );
		}

	//Finally destroy the hlist	 (should just be one big block)
	//free( hlist	);  
		
	//Free the Gumbo and Table structures
	gumbo_destroy_output( &kGumboDefaultOptions, output );
	//lt_free( &t );

	//???
	return 0; 

}
