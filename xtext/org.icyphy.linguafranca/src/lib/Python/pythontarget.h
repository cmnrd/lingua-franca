/**
 * @file
 * @author Edward A. Lee (eal@berkeley.edu)
 * @author Soroush Bateni (soroush@utdallas.edu)
 *
 * @section LICENSE
Copyright (c) 2020, The University of California at Berkeley.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * @section DESCRIPTION
 * Target-specific runtime functions for the Python target language.
 * This API layer can be used in conjunction with:
 *     target Python;
 * 
 * Note for target language developers. This is one way of developing a target language where 
 * the C core runtime is adopted. This file is a translation layer that implements Lingua Franca 
 * APIs which interact with the internal _lf_SET and _lf_schedule APIs. This file can act as a 
 * template for future runtime developement for target languages.
 * For source generation, see xtext/org.icyphy.linguafranca/src/org/icyphy/generator/PythonGenerator.xtend.
 */

#ifndef PYTHON_TARGET_H
#define PYTHON_TARGET_H


#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <structmember.h>
#include "ctarget.h"
#include "core/reactor.c"

#ifdef _MSC_VER
#include <../include/limits.h>
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <limits.h>
#endif

// MODULE_NAME is expected to be defined in the main file of the generated code
#ifndef MODULE_NAME
#define MODULE_NAME LinguaFranca_Default
#endif

#define CONCAT(x,y) x##y
#define GEN_NAME(x,y) CONCAT(x,y)
#define STRINGIFY(X) #X
#define TOSTRING(x) STRINGIFY(x)

/**
 * The struct used to instantiate a port
 * in Lingua Franca. This template is used 
 * in the PythonGenerator instead of redefining
 * a struct for each port.
 * This template can be used for both primitive types
 * and statically allocated arrays (e.g., int x[3];).
 * T value: the value of the port with type T
 * is_present: indicates if the value of the port is present
 *     at the current logcal time
 * num_destinations: used for reference counting the number of
 *     connections to destinations.
 **/
typedef struct {
    PyObject_HEAD
    PyObject* value;
    bool is_present;
    int num_destinations;
} generic_port_instance_struct;

/**
 * Special version of the template_input_output_port_struct
 * for dynamic arrays
 **/
typedef struct {
    PyObject_HEAD
    PyObject* value;
    bool is_present;
    int num_destinations;
    token_t* token;
    int length;
} generic_port_instance_with_token_struct;

//////////////////////////////////////////////////////////////
/////////////  SET Functions (to produce an output)

/**
 * Set the specified output (or input of a contained reactor)
 * to the specified value.
 *
 * This version is used for primitive types such as int,
 * double, etc. as well as the built-in types bool and string.
 * The value is copied and therefore the variable carrying the
 * value can be subsequently modified without changing the output.
 * This can also be used for structs with a type defined by a typedef
 * so that the type designating string does not end in '*'.
 * @param self Pointer to the calling object.
 * @param args contains: 
 *      @param out The output port (by name) or input of a contained
 *                 reactor in form input_name.port_name.
 *      @param val The value to insert into the port struct.
 */
static PyObject* py_SET(PyObject *self, PyObject *args)
{
    generic_port_instance_struct* port;
    PyObject* val;

    if (!PyArg_ParseTuple(args, "OO" ,&port, &val))
        return NULL;
    
    Py_DECREF(port->value);
    _LF_SET(port, val);
    Py_INCREF(port->value);

    Py_INCREF(Py_None);
    return Py_None;
}

//////////////////////////////////////////////////////////////
///////////// Main function callable from Python code
static PyObject* py_main(PyObject *self, PyObject *args)
{
    main(1, NULL);

    Py_INCREF(Py_None);
    return Py_None;
}

//////////////////////////////////////////////////////////////
/////////////  Python Helper Structs

/*
 * The members of a port_instance, used to define
 * a native Python type.
 */
static PyMemberDef port_instance_members[] = {
    {"value", T_OBJECT, offsetof(generic_port_instance_struct, value), 0, "Value of the port"},
    {"is_present", T_BOOL, offsetof(generic_port_instance_struct, is_present), 0, "Check if value is present at current logical time"},
    {"num_destinations", T_INT, offsetof(generic_port_instance_struct, num_destinations), 0, "Number of destinations"},
    {NULL}  /* Sentinel */
};

/*
 * The definition of port_instance type object.
 * Used to describe how port_instance behaves.
 */
static PyTypeObject port_instance_t = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "LinguaFranca.port_instance",
    .tp_doc = "port_instance objects",
    .tp_basicsize = sizeof(generic_port_instance_struct),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_members = port_instance_members,
};

/*
 * The members of a port_instance, used to define
 * a native Python type.
 */
static PyMemberDef port_instance_token_members[] = {
    {"value", T_OBJECT, offsetof(generic_port_instance_with_token_struct, value), 0, "Value of the port"},
    {"is_present", T_BOOL, offsetof(generic_port_instance_with_token_struct, is_present), 0, "Check if value is present at current logical time"},
    {"num_destinations", T_INT, offsetof(generic_port_instance_with_token_struct, num_destinations), 0, "Number of destinations"},
    {NULL}  /* Sentinel */
};

/*
 * The definition of port_instance type object.
 * Used to describe how port_instance behaves.
 */
static PyTypeObject port_instance_token_t = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "LinguaFranca.port_instance",
    .tp_doc = "port_instance objects",
    .tp_basicsize = sizeof(generic_port_instance_with_token_struct),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_members = port_instance_token_members,
};

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef GEN_NAME(MODULE_NAME,_methods)[] = {
  {"start", py_main, METH_VARARGS, NULL},
  {"SET", py_SET, METH_VARARGS, NULL},
  {NULL, NULL, 0, NULL}
};

static PyModuleDef MODULE_NAME = {
    PyModuleDef_HEAD_INIT,
    TOSTRING(MODULE_NAME),
    "LinguaFranca Python Module",
    -1,
    GEN_NAME(MODULE_NAME,_methods)
};

//////////////////////////////////////////////////////////////
/////////////  Python Helper Functions

/* 
 * 
 */
static PyObject* invoke_python_function(string module, string class, string func, PyObject* pArgs)
{

    // Set if the interpreter is already initialized
    int is_initialized = 0;

    if(Py_IsInitialized())
    {
        is_initialized = 1;
    }

#ifdef VERBOSE
    printf("Starting the function start()\n");
#endif

    // Necessary PyObject variables to load the react() function from test.py
    PyObject *pFileName, *pModule, *pDict, *pClass, *pFunc;

    PyObject *rValue;

    // Initialize the Python interpreter
    Py_Initialize();

#ifdef VERBOSE
    printf("Initialized the Python interpreter.\n");
#endif
    
    // Decode the MODULE name into a filesystem compatible string
    pFileName = PyUnicode_DecodeFSDefault(module);
    
    // Set the Python search path to be the current working directory
    char cwd[PATH_MAX];
    if( getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(stderr, "Failed to get the current working directory.\n");
        exit(0);
    }

    wchar_t wcwd[PATH_MAX];

    mbstowcs(wcwd, cwd, PATH_MAX);

    Py_SetPath(wcwd);

#ifdef VERBOSE
    printf("Loading module %s in %s.\n", module, cwd);
#endif

    pModule = PyImport_Import(pFileName);

#ifdef VERBOSE
    printf("Loaded module %p.\n", pModule);
#endif

    // Free the memory occupied by pFileName
    Py_DECREF(pFileName);

    // Check if the module was correctly loaded
    if (pModule != NULL) {
        // Get contents of module. pDict is a borrowed reference.
        pDict = PyModule_GetDict(pModule);
        if(pDict == NULL)
        {
            PyErr_Print();
            fprintf(stderr, "Failed to load contents of module %s", module);
            return 1;
        }

        // Get the class
        pClass = PyDict_GetItemString(pDict, class);
        if(pClass == NULL){
            PyErr_Print();
            fprintf(stderr, "Failed to load class %s in module %s", class, module);
            return 1;
        }

        //Py_DECREF(pDict);

#ifdef VERBOSE
        printf("Loading function %s.\n", func);
#endif
        // Get the function react from test.py
        pFunc = PyObject_GetAttrString(pClass, func);

#ifdef VERBOSE
        printf("Loaded function %p.\n", pFunc);
#endif


        // Check if the funciton is loaded properly
        // and if it is callable
        if (pFunc && PyCallable_Check(pFunc))
        {
#ifdef VERBOSE
            printf("Attempting to call function.\n");
#endif


            // Call the func() function with arguments pArgs
            // The output will be returned to rValue
            rValue = PyObject_CallObject(pFunc, pArgs);

#ifdef VERBOSE
                printf("Finished calling %s.\n", func);
#endif

            // Check if the function is executed correctly
            if (rValue != NULL)
            {
#ifdef VERBOSE
                printf("I called %s and got %ld.\n", func , PyLong_AsLong(rValue));
#endif
                Py_DECREF(rValue);
            }
            else {
                // If the function call fails, print an error
                // message and get rid of the PyObjects
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                Py_DECREF(pClass);
                PyErr_Print();
                fprintf(stderr, "Calling react failed.\n");
                exit(0);
            }


            // Free pArgs (or rather decrement its reference count)
            //Py_DECREF(pArgs);
        }
        else
        {
            // Function is not found or it is not callable
            if (PyErr_Occurred())
            {
                PyErr_Print();
            }
            fprintf(stderr, "Function %s was not found or is not callable.\n", func);
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule); 
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", module);
    }
    
#ifdef VERBOSE
    printf("Done with start()\n");
#endif

    if(is_initialized == 0)
    {
        /* We are the first to initilize the Pyton interpreter. Destroy it when done. */
        Py_FinalizeEx();
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/*
 * Python calls this to let us initialize our module
 */
PyMODINIT_FUNC
GEN_NAME(PyInit_,MODULE_NAME)(void)
{
    PyObject *m;

    // Initialize the port_instance type
    if (PyType_Ready(&port_instance_t) < 0)
        return NULL;

    // Initialize the port_instance_token type
    if (PyType_Ready(&port_instance_token_t) < 0)
        return NULL;

    m = PyModule_Create(&MODULE_NAME);

    if (m == NULL)
        return NULL;

    // Add the port_instance type to the module's dictionary
    Py_INCREF(&port_instance_t);
    if (PyModule_AddObject(m, "port_instance", (PyObject *) &port_instance_t) < 0) {
        Py_DECREF(&port_instance_t);
        Py_DECREF(m);
        return NULL;
    }

    // Add the port_instance_token type to the module's dictionary
    Py_INCREF(&port_instance_token_t);
    if (PyModule_AddObject(m, "port_instance_token", (PyObject *) &port_instance_token_t) < 0) {
        Py_DECREF(&port_instance_token_t);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

#endif // PYTHON_TARGET_H