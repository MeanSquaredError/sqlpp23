[**\< Recipes**](/docs/recipes.md)

# Thread-local connections

As of the time of this writing, database connections created by the three main databases, supported by sqlpp23 (MySQL, PostgreSQL, SQLite3), are not thread-safe. The sqlpp23 library is thread-agnostic which means that it does not add any requirements or guarantees to the thread-safety of the underlying database objects and operations. So it is up to the library user to ensure the thread-safety of the database operations performed through these database connections.

One possible way of making thread-safe database queries is by using a separate database connection for each thread. The [provided sample implementation](/tests/postgresql/recipes/thread_local_connection.cpp) uses a global database connection object (g_dbc) that mimics the regular database connections provided by sqlpp23 and lets the user execute database queries via operator(), pretty much like regular sqlpp23 database connection do. Internally this global object assigns a regular database connection to each user thread, so what the user tries to execute an SQL query through the global object, it just forwards the query to the database connection assigned to the calling thread. These per-thread connections are not created immediately after a new thread is created. Instead a database connection is created and assigned to a thread the first time when that thread tries to execute a database query through the global object.

In our implementation of the global connection object we have chosen to offload all the thread-related chores to the C++ compiler and runtime by adding thread_local to our object's definition. This instructs the C++ runtime that each thread should get it own, separate instance of the global connection object. This approach greatly simplifies the implementation of our global connection object, in exchange forcing us to declare it as global, since thread_local cannot be applied to







The per-thread connections are stored in a map, where the thread ids serve as keys and database connections are the values. When the user tries to execute a query, tls_connection checks if the current thread already has a database connection assigned to it. If the thread still does not have a database connection, a new connection is fetched from a thread pool and assigned to the current thread. Then the database connection of the current thread is used to execute the query and its result is returned to the user.

This approach works quite well for programs that

[**\< Recipes**](/docs/recipes.md)
