[**\< Recipes**](/docs/recipes.md)

# Thread-safety in the database connectors

As of the time of this writing, database connections created by the three main databases, supported by sqlpp23 (MySQL, PostgreSQL, SQLite3), are not thread-safe. The sqlpp23 library is thread-agnostic which means that it does not add any requirements or guarantees to the thread-safety of the underlying database objects and operations. So it is up to the library user to ensure the thread-safety of the database operations performed through these database connections.

# Making thread-safe queries using thread-unsafe connectors

Our approach is based on a couple of simple observations

- It is possible to use thread-unsafe connections in a thread-safe


The easiest way to
In this document we provide a simple approach to making your database code thread-safe by using thread-locate database connections.



possible way of making thread-safe database queries is by using a separate database connection for each thread. We define a class called


 uses a global function object (g_dbc) that mimics the regular database connections provided by sqlpp23 and lets the user execute database queries, pretty much like regular sqlpp23 database connection do. Internally g_dbc a regular database connection to each user thread, so when the user tries to execute an SQL query, g_dbc just forwards the query to the database connection assigned to the calling thread. These per-thread connections are not created immediately after a new thread is created. Instead a database connection is created and assigned to a thread the first time when that thread tries to execute a database query through g_dbc.

In our implementation of g_dbc we have chosen to offload all the thread-related chores to the C++ compiler and runtime by adding the thread_local storage class to g_dbc's definition. Effectively this provides each thread with its own, separate instance of g_dbc, stored in the program's Thread Local Storage (TLS). Having the compiler take care of t




. This approach greatly simplifies the implementation of g_dbc, in exchange forcing us to declare it as global, since thread_local cannot be applied to







The per-thread connections are stored in a map, where the thread ids serve as keys and database connections are the values. When the user tries to execute a query, tls_connection checks if the current thread already has a database connection assigned to it. If the thread still does not have a database connection, a new connection is fetched from a thread pool and assigned to the current thread. Then the database connection of the current thread is used to execute the query and its result is returned to the user.

This approach works quite well for programs that



The [provided sample implementation](/tests/postgresql/recipes/thread_local_connection.cpp)

[**\< Recipes**](/docs/recipes.md)
