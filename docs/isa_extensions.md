# Arrays
A program may request that an array is created. This allows multiple invoked functions to use a 
single persistent memory.

The steps to create an array are roughly:
1. Allocate an array of some requested size + 1.
2. Use the 0'th element to store the array size.
3. Store a pointer to the array inside a mapped array.
4. Return an array reference.

A mapped array consists of two arrays, one for storing values and another for storing a flag 
(called a map). If a flag is set at some index ```n``` in the map, then a value at index ```n```, 
namely a pointer to an array, also exists. This will be important in the discussion about the 
garbage collector.
Once an array is created, it is easy to imagine an interface that sets and gets elements from the 
array, while being able to check bounds and if the array exists by checking the corresponding 
flag in the map.

## Array References
Each element on the stack is a 32-bit signed integer thus reserving values for array references is 
simply not an option because some programs may require the full range of values. The solution to 
the problem became a special pattern that can be searched for in memory. The pattern is 
```0xAA?????A``` where the question marks are replaced by a 24-bit unsigned integer. 
This means that array references are any number between ```0xAAFFFFFA``` and ```0xAA00000A``` 
(inclusive). With this setup, the program may have at most 2^24 = 16777216 arrays which is fairly 
reasonable.

It may appear as if the range from ```0xAAFFFFFA``` to ```0xAA00000A``` is reserved but in fact, 
the program can freely use any of those values without affecting arrays. The only reason a special 
pattern is used is to distinguish elements in memory. Unfortunately, if a program uses many of 
these values, the garbage collector will not be effective in clearing up memory but this is still 
better than reserving values (this will be discussed in the next section).

Another important point about the 24-bit number identifying an array is that it's directly 
connected with the index in the mapped array where the array pointer is stored. This means that 
after creating an array, if the first unclaimed spot in the mapped array is at index 85, then we 
store the array pointer at index 85 in the values array, set the flag at index 85 to true and, 
return an array reference of ```0xAA00055A```.

## Garbage Collector
Given the array implementation above, we have all we need to create a reasonably fast garbage 
collector. The GC works as follows:

1. Loop over the stack and apply a bitwise AND with ```0xFF00000F```, followed by a bitwise XOR 
with ```0xAA00000A```, to determine if the element at least looks like an array reference.
2. The 24-bit number identifying an array is extracted and used as an index in the map 
(inside the mapped array) to check if such an array exists, and if it does we mark this index as 
reachable.
3. Loop over elements of all marked arrays repeating steps 1 and 2 to determine if an element 
inside an array stores an array reference to another array that should also be marked.
4. Remove all arrays that were not marked.

The garbage collector is run every 100 array creations, when resizing the stack and reaching an 
out of memory error, resizing a mapped array and reaching an out of memory error, when 
duplicating strings, and when the ```GC``` instruction is executed.

Of course, a regular number can have the same value as an array reference in which case the 
garbage collector will falsely assume an array should not be removed when it should be. This will 
lead to inefficient utilization of memory as some garbage will not be removed but the behavior 
apparent to the user will remain the same.


# Networking
Networking is implemented in the same way as arrays because both use mapped arrays which also 
means they share the majority of their code. Network references are also created in the same 
way as array references but instead of using the ```0xAA?????A``` pattern, it instead uses 
```0xCC?????C```. This means that theoretically the VM can support 2^24 = 16777216 sockets but 
the maximum on Linux for example is only 2^16 = 65535 so this limit will likely never be a 
problem. 
