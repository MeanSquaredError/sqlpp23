[**\< Index**](/docs/README.md)

# Optimistic Concurrency Control

## What is Optimistic Concurrency Control?

Optimistic Concurrency Control (OCC), also known as Optimistic Locking, is a method for handling concurrent access to shared resources, that runs concurrent transactions without any explicit locks and relies on the underlying database for the detection and resolution of any conflicts arising from the access to the shared resources. If automatic resolution of a conflict fails, then one of the conflicting transactions is aborted and retried one or more times, until it succeeds.

The lack of explicit locking allows OCC to provide high performance if there is little contention between the parallel transactions. On the other hand, if  there is high contention, the performance may (and likely will) degrade significantly.

## When should Optimistic Concurrency Control be used?

The following table compares Optimistic Concurrency Control (OCC) and Pessimistic Concurrency Control (PCC):

|                                | OCC     | PCC    |
|--------------------------------|---------|--------|
| Uses transactions              | Yes     | Yes    |
| Retries transactions           | Yes     | No     |
| Uses locks                     | No[^1]  | Yes    |
| Performance on low contention  | High    | Medium |
| Performance on high contention | Low     | Medium |
| Easy to reason about and use   | Yes[^2] | No     |

[^1]: The underlying database may lock implicitly some rows or tables.
[^2]: May require some knowledge of the database-specific serialization failures.

## How to implement Optimistic Concurrency Control?

The common implementation pattern for OCC usually goes along the lines of
