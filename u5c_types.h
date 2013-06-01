/* 
 * u5c defintions
 */

struct u5c_type;
struct u5c_data;
struct u5c_component; /* forward declaration */


/* serialization */
typedef struct u5c_serialization {
	const char* name;		/* serialization name */
	const char* type;		/* serialization type */\

	int(*serialize)(struct u5c_data*, char* buffer, uint32_t max_size); 
	int(*deserialize)(void*, struct u5c_data*);
	UT_hash_handle hh;
} u5c_serialization_t;


/* type and value (data) */

enum {
	TYPE_CLASS_STRUCT,	/* simple consecutive struct */
	TYPE_CLASS_CUSTOM	/* requires custom serialization */
};

typedef struct u5c_type {
	const char* name;		/* name: dir/header.h/struct foo*/
	uint32_t class;			/* CLASS_STRUCT=1, CLASS_CUSTOM, CLASS_FOO ... */
	u5c_serialization_t* serializations;
	UT_hash_handle hh;
} u5c_type_t;

typedef struct u5c_data {
	const u5c_type_t* type;	/* link to u5c_type */
	void* data;
	uint32_t size;
} u5c_data_t;


/* Port attributes */
enum {
	PORT_DIR_IN    =	1 << 0,
	PORT_DIR_OUT   =	1 << 1,
};

enum {
	PORT_ACTIVE = 1 << 0,
};


/* return values */
enum {
	PORT_READ_NODATA   = 1,
	PORT_READ_NEWDATA  = 2,

	/* ERROR conditions */
	EPORT_INVALID       = -1,
	EPORT_INVALID_TYPE  = -2,
	
	/* Registration, etc */
	EINVALID_BLOCK_TYPE = -3,
	ENOSUCHBLOCK        = -4,
	EALREADY_REGISTERED = -5,
};

/* Port
 * no distinction between type and value
 */
typedef struct u5c_port {
	char* name;		/* name of port */
	char* in_type_name;	/* string data type name */
	char* out_type_name;	/* string data type name */

	u5c_type_t* in_type;    	/* filled in automatically */
	u5c_type_t* out_type;    	/* filled in automatically */

	char* meta_data;		/* doc, etc. */
	uint32_t attrs;			/* FP_DIR_IN or FP_DIR_OUT */
	uint32_t state;			/* active/inactive */

	/* uint32_t(*read)(struct u5c_port* port, u5c_data_t* value); */
	/* void(**write)(struct u5c_port* port, u5c_data_t* value);          /\* many target interactions, end with NULL *\/ */

	struct u5c_component* in_interaction;
	struct u5c_component** out_interaction;

	/* statistics */
	uint32_t stat_writes;
	uint32_t stat_reades;
	
	/* todo time stats */
} u5c_port_t;


/*
 * u5c configuration
 */
typedef struct u5c_config {
	const char* name;
	const char* type_name;
	void *data;
} u5c_config_t;

/* 
 * u5c component
 */

enum {
	BLOCK_TYPE_COMPUTATION,
	BLOCK_TYPE_INTERACTION,
	BLOCK_TYPE_TRIGGER,
};

/* component definition */
typedef struct u5c_component {
	char* name;		/* type name */
	char* meta_data;	/* doc, etc. */
	char* type;		/* type, (computation, interaction, trigger) */

	u5c_port_t* ports;
	const u5c_config_t* configs;

	int(*init) (struct u5c_component*);
	int(*start) (struct u5c_component*);
	void(*stop) (struct u5c_component*);
	void(*cleanup) (struct u5c_component*);

	/* type dependent component methods */
	union {
		/* COMP_TYPE_COMPUTATION */
		void(*step) (struct u5c_component*);
	
		/* COMP_TYPE_INTERACTION */
		struct {
			/* read and write: these are implemented by interactions and
			 * called by the ports read/write */
			uint32_t(*read)(struct u5c_component* interaction, u5c_data_t* value);
			void(*write)(struct u5c_component* interaction, u5c_data_t* value);
		};
		
		/* COMP_TYPE_TRIGGER - no special ops */
	};


	char *prototype; /* name of prototype, NULL if none */

	/* statistics, todo step duration */
	uint32_t stat_num_steps;

	void* private_data;

	UT_hash_handle hh;

} u5c_component_t;


/* process global data
 * This contains arrays of known component types, types
 */
typedef struct u5c_node_info {
	const char *name;

	u5c_component_t *components;	 	/* known component types */
	u5c_component_t *interactions;	/* known interaction types */
	u5c_component_t *triggers;		/* known trigger types */
	u5c_type_t *types;			/* known types */
	
} u5c_node_info_t;


/* 
 * runtime API 
*/


/* initalize a node: typically used by a main */
int u5c_node_init(u5c_node_info_t* ni);
void u5c_node_cleanup(u5c_node_info_t* ni);

int u5c_num_components(u5c_node_info_t* ni);


/* register/unregister different entities */
int u5c_computation_register(u5c_node_info_t *ni, u5c_component_t* comp);
u5c_component_t* u5c_computation_unregister(u5c_node_info_t *ni, const char* name);

int u5c_type_register(u5c_node_info_t* ni, u5c_type_t* type);
int u5c_type_unregister(u5c_node_info_t* ni, u5c_type_t* type);

int u5c_register_interaction(u5c_node_info_t* ni, u5c_component_t* inter);
void u5c_unregister_interaction(u5c_node_info_t* ni, u5c_component_t* inter);

int u5c_register_trigger(u5c_node_info_t* ni, u5c_component_t* trig);
int u5c_unregister_trigger(u5c_node_info_t* ni, u5c_component_t* trig);

u5c_component_t* u5c_computation_create(u5c_node_info_t* ni, const char *type, const char* name);

int u5c_component_destroy(u5c_node_info_t *ni, char *name);

int u5c_connect(u5c_component_t* comp1, u5c_component_t* comp2, u5c_component_t* ia);
int u5c_disconnect(u5c_component_t* comp1, const char* portname);


/* intra-component  API */
u5c_port_t* u5c_port_get(u5c_component_t* comp, const char* name);
u5c_port_t* u5c_port_add(u5c_component_t* comp);
u5c_port_t* u5c_port_rm(u5c_component_t* comp);

uint32_t __port_read(u5c_port_t* port, u5c_data_t* res);
void __port_write(u5c_port_t* port, u5c_data_t* res);

/* module init/cleanup */
int __initialize_module(u5c_node_info_t* ni);
void __cleanup_module(u5c_node_info_t* ni);
