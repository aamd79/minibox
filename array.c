It manages the memory resources needed to create and add resources in the objects and array.
The types of data handled by the API are described below.

NULL:
Null object, for example, to indicate the end of the collection.
The MiniBox API saves this value as null = 0.

BOOLEAN:
Typical true and false values ​​of c.
The MiniBox API saves this value as true = 1, false = 0.

NUMBER:
All numeric values ​​will be of type double.
In the representations, the MiniBox API will determine whether to add the decimal part or only the entire part.
Examples: (1.0 = 1) (3.900 = 3.9)

STRING:
UTF-8 character string ending in zero of c.
For the dynamic memory that you want to discard, we will add the
MINIBOX_MEMORY_RELEASE parameter.
If we do not want the MiniBox API to free the memory or this is a constant, we will add the MINIBOX_MEMORY_RETAINT parameter.
Example: "This is an example of a string".

ARRAY and OBJECT:
Like STRING, it will be necessary to indicate to the MiniBox API how the memory will be managed.
MINIBOX_MEMORY_RELEASE will remove the object and check its contents if it should also be deleted.
MINIBOX_MEMORY_RETAINT keeps the object in memory.
