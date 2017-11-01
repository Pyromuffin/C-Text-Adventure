//
// Created by Kelly MacNeill on 5/3/17.
//

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "IndexVector.h"

static const uint MAX_VECTORS = 100;

static DynamicIndexArray s_AllVectors[MAX_VECTORS]; // this is only for tracking purposes, to make sure we're not leaking.
MAKE_STATIC_VECTOR(s_FreeList, MAX_VECTORS);

void PushIndexStatic(DynamicIndexArray *me, IndexType handle) {
    assert(me->length <  me->capacity);
    assert(me->capacity > 0);
    me->handles[me->length] = handle;
    me->length++;
}

void ResizeVector( DynamicIndexArray* me, uint newCapacity )
{
    //printf("Resizing vector from %d to %d, length %d\n", me->capacity, newCapacity, me->length);
    //@todo perhaps consider the ability to get smaller.
    assert(newCapacity > me->capacity);
    IndexType* newStorage = (IndexType*) malloc(sizeof(IndexType) * newCapacity);

    //I sure hope me is right??????
    if((me->capacity > 0) && (me->handles != NULL))
    {
        memcpy(newStorage, me->handles, me->length * sizeof(IndexType));
        free(me->handles);
    }
	me->capacity = newCapacity;
	me->handles = newStorage;
}

void PushIndex(DynamicIndexArray *me, IndexType index)
{
    if(me->capacity <= me->length )
    {
        ResizeVector(me, me->capacity*2);
    }

	me->handles[me->length] = index;
	me->length++;
}

IndexType PopIndex(DynamicIndexArray *me)
{
    assert(me->length > 0);
    assert(me->capacity > 0);
    me->length--;
    return me->handles[me->length];
}

DynamicIndexArray* AllocateIndexVector(uint capacity, const char* name)
{
    IndexType freeVector = PopIndex(&s_FreeList);
    DynamicIndexArray* vector = &s_AllVectors[freeVector];
    vector->name = name;
    assert(vector->capacity == 0);
    ResizeVector(vector, capacity);

    //printf("alloc index %d\n", freeVector);

    return vector;
}

void FreeIndexVector(DynamicIndexArray* vector)
{
    assert(vector->handles);
    assert(vector->capacity > 0);
    free(vector->handles);
    vector->capacity = 0;
    vector->length = 0;
    vector->handles = NULL;

    IndexType offset = (IndexType)(vector - &s_AllVectors[0]); // don't forget about how pointer arithmetic works.
    assert(offset >= 0);
    assert(offset < MAX_VECTORS);

    //printf("freed index %d\n", offset);
    PushIndexStatic( &s_FreeList, offset );
}

void RemoveSwapBack(DynamicIndexArray *me, IndexType index)
{
	assert(me->length > 0);
	assert(index < me->length);

	IndexType back = me->handles[me->length -1];
	me->handles[index] = back;
	me->length--;
}

void InitVectorTracking()
{
    for(uint i = 0; i < MAX_VECTORS; i++)
    {
        PushIndexStatic(&s_FreeList, (IndexType)i);
    }
}

void CheckForVectorLeaks()
{
    bool foundIndices[MAX_VECTORS];
    memset(foundIndices, 0, sizeof(foundIndices));

    for( int i =0; i < MAX_VECTORS; i ++)
    {
        IndexType index = PopIndex(&s_FreeList);
        if(foundIndices[index])
        {
            printf("Double freed index %d, position %d, vector %s\n", index, i, s_AllVectors[index].name);
        }

        foundIndices[index] = true;
    }

    for( int i =0; i < MAX_VECTORS; i ++)
    {
        if(!foundIndices[i])
        {
            printf("Leaked Index %d, vector %s\n", i, s_AllVectors[i].name);
        }
    }

	assert(s_FreeList.length == MAX_VECTORS);
}


void DeduplicateIndices(DynamicIndexArray * me)
{
	for (uint handleIndex = 0; handleIndex < me->length; handleIndex++)
	{
		// for each element, scan through the rest of the elements to check for duplicates.
		IndexType handle = me->handles[handleIndex];

		for (uint remainingHandleIndex = handleIndex + 1; remainingHandleIndex < me->length; remainingHandleIndex++)
		{
			IndexType comparedHandle = me->handles[remainingHandleIndex];
			if (handle == comparedHandle)
			{
				// remove that element by swapping with the back.
				RemoveSwapBack(me, remainingHandleIndex);
				// and then decrement remaining ref index so that we check that index again (because we changed the value there).
				remainingHandleIndex--;
			}
		}
	}

}

void ResetVetor(DynamicIndexArray * me)
{
	me->length = 0;
}
