/******************************************************************************
 * File: gs.h
 * Created: 2016-07-14
 * Last Updated: 2016-08-23
 * Creator: Aaron Oman (a.k.a GrooveStomp)
 * Notice: (C) Copyright 2016 by Aaron Oman
 *-----------------------------------------------------------------------------
 *
 * Standard library for personal use. Heavily influenced by Sean Barrett's stb
 * and Casey Muratori's Handmade Hero.
 *
 ******************************************************************************/
#ifndef GS_VERSION
#define GS_VERSION 0.3.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <stdarg.h> /* va_list */
#include <libgen.h> /* basename */

#define GSArraySize(Array) (sizeof((Array)) / sizeof((Array)[0]))

/******************************************************************************
 * Usage:
 *
 * int Numbers[] = { 1, 2, 3, 4, 5 };
 * GSArrayForEach(int *Number, Numbers)
 * {
 *         printf("Number[%i]: %i\n", Index, *Number);
 * }
 *
 * NOTE:
 * The variable `Index' is automatically generated for you.
 * `Item' must be a pointer to the type of variable used in the Array.
 *
 * Implementation taken from: http://stackoverflow.com/a/400970
 ******************************************************************************/
#define GSArrayForEach(Item, Array) \
        for(int Keep##__LINE__ = 1, \
                Count##__LINE__ = 0, \
                Index = 0, \
                Size##__LINE__ = sizeof((Array)) / sizeof(*(Array)); \
            Keep##__LINE__ && Count##__LINE__ != Size##__LINE__; \
            Keep##__LINE__ = !Keep##__LINE__, Count##__LINE__++) \
                for(Item = (Array) + Count##__LINE__; Keep##__LINE__; Keep##__LINE__ = !Keep##__LINE__, Index++)

#define GSMax(A, B) ((A) < (B) ? (B) : (A))
#define GSMin(A, B) ((A) < (B) ? (A) : (B))

#define GSAbortWithMessage(...) \
        { \
                char String##__LINE__[256];                             \
                sprintf(String##__LINE__, "In %s() at line #%i: ", __func__, __LINE__); \
                fprintf(stderr, "%s", String##__LINE__);                 \
                fprintf(stderr, __VA_ARGS__); \
                exit(EXIT_FAILURE); \
        }

#define GS1024Inverse 1.0/1024
#define GSBytesToKilobytes(X) (X) * GS1024Inverse
#define GSBytesToMegabytes(X) GSBytesToKilobytes((X)) * GS1024Inverse
#define GSBytesToGigabytes(X) GSBytesToMegabytes((X)) * GS1024Inverse

#define GSNullChar '\0'
#define GSNullPtr NULL

/******************************************************************************
 * Boolean Definitions
 ******************************************************************************/

typedef int gs_bool;
#ifndef false
#define false 0
#define true !false
#endif

/******************************************************************************
 * Character Definitions
 *-----------------------------------------------------------------------------
 * Functions to interact with C's basic ASCII char type.
 ******************************************************************************/

gs_bool
GSCharIsEndOfStream(char C)
{
	return(C == '\0');
}

gs_bool
GSCharIsEndOfLine(char C)
{
	return((C == '\n') ||
	       (C == '\r'));
}

gs_bool
GSCharIsWhitespace(char C)
{
	return((C == ' ') ||
	       (C == '\t') ||
	       (C == '\v') ||
	       (C == '\f') ||
	       GSCharIsEndOfLine(C));
}

gs_bool
GSCharIsOctal(char C)
{
	gs_bool Result = (C >= '0' && C <= '7');
	return(Result);
}

gs_bool
GSCharIsDecimal(char C)
{
	gs_bool Result = (C >= '0' && C <= '9');
	return(Result);
}

gs_bool
GSCharIsHexadecimal(char C)
{
	gs_bool Result = ((C >= '0' && C <= '9') ||
		       (C >= 'a' && C <= 'f') ||
		       (C >= 'A' && C <= 'F'));
	return(Result);
}

gs_bool
GSCharIsAlphabetical(char C)
{
	gs_bool Result = ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z'));
	return(Result);
}

gs_bool
GSCharIsAlphanumeric(char C)
{
        gs_bool Result = GSCharIsAlphabetical(C) || GSCharIsDecimal(C);
        return(Result);
}

gs_bool
GSCharIsUpcase(char C)
{
        gs_bool Result =
                GSCharIsAlphabetical(C) &&
                (C >= 'A') &&
                (C <= 'Z');

        return(Result);
}

char
GSCharUpcase(char C)
{
        char Result = C;
        if(GSCharIsAlphabetical(C) &&
           (C >= 'a' && C <= 'z'))
        {
                int Delta = C - 'a';
                Result = Delta + 'A';
        }

        return(Result);
}

gs_bool
GSCharIsDowncase(char C)
{
        gs_bool Result =
                GSCharIsAlphabetical(C) &&
                (C >= 'a') &&
                (C <= 'z');

        return(Result);
}

char
GSCharDowncase(char C)
{
        char Result = C;
        if(GSCharIsAlphabetical(C) &&
           (C >= 'A' &&C <= 'Z'))
        {
                int Delta = C - 'A';
                Result = Delta + 'a';
        }

        return(Result);
}

/******************************************************************************
 * String Definitions
 *-----------------------------------------------------------------------------
 * C string type. That is, ASCII characters with terminating NULL.
 ******************************************************************************/

gs_bool
GSStringIsEqual(char *LeftString, char *RightString, int MaxNumToMatch)
{
	int NumMatched = 0;

        if((*LeftString == GSNullChar) ||
           ((*RightString == GSNullChar) &&
            (*LeftString != *RightString)))
        {
                return(false);
        }

	while(NumMatched < MaxNumToMatch)
	{
		if(*LeftString == *RightString)
		{
			LeftString++;
			RightString++;
			NumMatched++;
		}
		else
		{
			return(false);
		}
	}

	return(true);
}

size_t
GSStringLength(char *String)
{
	char *P = String;
	while(*P != '\0') P++;
	return(P - String);
}

gs_bool
GSStringCopy(char *Source, char *Dest, int Max)
{
        if(Source == NULL || Dest == NULL)
        {
                return(false);
        }

        int I = 0;
        for(; Source[I] != '\0' && I < Max; I++)
        {
                Dest[I] = Source[I];
        }
        Dest[I] = '\0';

        return(true);
}

gs_bool
GSStringCopyNoNull(char *Source, char *Dest, int Max)
{
        if(Source == NULL || Dest == NULL)
        {
                return(false);
        }

        for(int I = 0; Source[I] != '\0' && I < Max; I++)
        {
                Dest[I] = Source[I];
        }

        return(true);
}

/* NOTE: Assumes a maximum string length of 512 bytes. */
unsigned int /* Returns number of bytes copied. */
GSStringTrimWhitespace(char *Source, unsigned int MaxLength)
{
        char Dest[512];
        MaxLength = GSMin(512, MaxLength);

        int FirstChar, LastChar;
        for(FirstChar = 0; GSCharIsWhitespace(Source[FirstChar]); FirstChar++);

        int StringLength = GSStringLength(Source);
        for(LastChar = StringLength - 1; GSCharIsWhitespace(Source[LastChar]); LastChar--);

        int Count = 0;
        for(int S=FirstChar; S<=LastChar && Count < MaxLength; Count++, S++)
        {
                Dest[Count] = Source[S];
        }

        for(int I=0; I<Count; I++)
        {
                Source[I] = Dest[I];
        }
        Source[Count] = GSNullChar;

        return(Count);
}

/*
  For any ascii character following an underscore, remove the underscore
  and capitalize the ascii char.
  This function assumes a maximum string size of 512 bytes.
  The first character is capitalized.
*/
unsigned int
GSStringSnakeCaseToCamelCase(char *Source, unsigned int SourceLength)
{
        char Dest[512]; /* Scratch buffer. */
        int Si = 0, Di = 0; /* Iterable indices for Source and Dest. */

        if((Source[Si] == '_') &&
           (Si+1 < SourceLength) &&
           GSCharIsAlphabetical(Source[Si+1]))
        {
                Si++;
        }
        Dest[Di] = GSCharUpcase(Source[Si]);
        Si++;
        Di++;

        SourceLength = GSMin(512, SourceLength);

        for(/*Si, Di*/; Si<SourceLength; Si++, Di++)
        {
                /* Replace any '_*' with 'upcase(*)' where * is an ascii char. */
                if((Source[Si] == '_') &&
                   (Si+1 < SourceLength) &&
                   GSCharIsAlphabetical(Source[Si+1]))
                {
                        Dest[Di] = GSCharUpcase(Source[Si+1]);
                        Si++;
                }
                /* Copy chars normally. */
                else
                {
                        Dest[Di] = Source[Si];
                }
        }

        /* Write the modified string back to source. */
        for(int I=0; I<Di; I++)
        {
                Source[I] = Dest[I];
        }
        Source[Di] = GSNullChar;

        return(Di);
}

/*
  Prerequisites:
  - Dest must be large enough to contain the modified string.

  For any Capitalized ascii character, replace with an underscore followed by
  the lowercase version of that character. This does not apply to leading char.
  eg.: CamelCase -> Camel_case
*/
unsigned int
GSStringCamelCaseToSnakeCase(char *Source, char *Dest, unsigned int SourceLength)
{
        int Si = 0, Di = 0; /* Iterable indices for Source and Dest. */
        Dest[Si] = GSCharDowncase(Source[Si]);
        Si++;
        Di++;

        for(/*Si, Di*/; Si<SourceLength && Source[Si] != GSNullChar; Si++, Di++)
        {
                /* Replace upcase ascii char with '_' and downcase ascii char. */
                if(GSCharIsUpcase(Source[Si]))
                {
                        Dest[Di] = '_';
                        Di++;
                        Dest[Di] = GSCharDowncase(Source[Si]);
                }
                /* Copy chars normally. */
                else
                {
                        Dest[Di] = Source[Si];
                }
        }
        Dest[Di] = GSNullChar;

        return(Di);
}

/*
  Capitalizes the first character found.
  Modifies Source in-place.
  Returns Source.
  eg.: hello -> Hello
       123foos -> 123Foos
*/
char *
GSStringCapitalize(char *Source, unsigned int Length)
{
        int Index = 0;

        while(true)
        {
                if(Index >= Length)
                        break;
                if(Source[Index] == GSNullChar)
                        break;
                if(GSCharIsAlphabetical(Source[Index]))
                        break;
                Index++;
        }

        if(Index >= Length)
                return(Source);

        Source[Index] = GSCharUpcase(Source[Index]);

        return(Source);
}

typedef gs_bool (*GSStringFilterFn)(char C);

int /* Returns length of new string */
GSStringKeep(char *Source, char *Dest, unsigned int MaxLength, GSStringFilterFn FilterFn)
{
        int SourceIndex = 0;
        int DestIndex = 0;

	while(SourceIndex < MaxLength)
	{
                if(FilterFn(Source[SourceIndex]))
                {
                        Dest[DestIndex] = Source[SourceIndex];
                        DestIndex++;
                }
                SourceIndex++;
        }
        Dest[DestIndex] = GSNullChar;

        return(DestIndex + 1);
}

int /* Returns length of new string */
GSStringReject(char *Source, char *Dest, unsigned int MaxLength, GSStringFilterFn FilterFn)
{
        int SourceIndex = 0;
        int DestIndex = 0;

	while(SourceIndex < MaxLength)
	{
                if(!FilterFn(Source[SourceIndex]))
                {
                        Dest[DestIndex] = Source[SourceIndex];
                        DestIndex++;
                }
                SourceIndex++;
        }
        Dest[DestIndex] = GSNullChar;

        return(DestIndex + 1);
}

/******************************************************************************
 * Hash Map
 *-----------------------------------------------------------------------------
 *
 * Usage:
 *     char *Value = "value";
 *     int StringLength = 256;
 *     int NumElements = 13;
 *     size_t BytesRequired = GSHashMapAllocSize(StringLength, NumElements);
 *     gs_hash_map *Map = GSHashMapInit(alloca(BytesRequired), StringLength, NumElements);
 *     GSHashMapSet(Map, "key", Value);
 *     if(GSHashMapHasKey(Map, "key"))
 *     {
 *         char *Result = (char *)GSHashMapGet(Map, "key");
 *         printf("Key(%s), Value(%s)\n", "key", Result);
 *     }
 ******************************************************************************/
typedef struct gs_hash_map
{
        unsigned int Count;
        size_t AllocatedBytes;
        unsigned int Capacity;
        unsigned int MaxKeyLength;

        char *Keys;
        void **Values;
} gs_hash_map;

unsigned int /* String must be a NULL-terminated string */
__GSHashMapComputeHash(gs_hash_map *Self, char *String)
{
        /*
          sdbm hash function: http://stackoverflow.com/a/14409947
        */
        unsigned int HashAddress = 0;
        for(unsigned int Counter = 0; String[Counter] != GSNullChar; Counter++)
        {
                HashAddress = String[Counter] +
                        (HashAddress << 6) +
                        (HashAddress << 16) -
                        HashAddress;
        }
        unsigned int Result = HashAddress % Self->Capacity;
        return(Result);
}

size_t
GSHashMapAllocSize(unsigned int MaxKeyLength, unsigned int NumEntries)
{
        int AllocSize =
                sizeof(gs_hash_map) +
                (sizeof(char) * MaxKeyLength * NumEntries) +
                (sizeof(void *) * NumEntries);
        return(AllocSize);
}

gs_hash_map *
GSHashMapInit(void *Memory, unsigned int MaxKeyLength, unsigned int NumEntries)
{
        gs_hash_map *Self = (gs_hash_map *)Memory;

        char *KeyValueMemory = (char *)Memory + sizeof(gs_hash_map);

        Self->MaxKeyLength = MaxKeyLength;
        Self->Capacity = NumEntries;
        Self->AllocatedBytes = GSHashMapAllocSize(MaxKeyLength, NumEntries);
        Self->Count = 0;

        int KeysMemLength = MaxKeyLength * NumEntries;

        Self->Keys = KeyValueMemory;
        memset(Self->Keys, 0, KeysMemLength);

        Self->Values = (void **)(Self->Keys + KeysMemLength);
        memset(Self->Values, 0, (sizeof(void **) * NumEntries));

        return(Self);
}

gs_bool
__GSHashMapUpdate(gs_hash_map *Self, char *Key, void *Value)
{
        unsigned int KeyLength = GSStringLength(Key);
        unsigned int HashIndex = __GSHashMapComputeHash(Self, Key);
        unsigned int StartHash = HashIndex;

        do
        {
                if(GSStringIsEqual(&Self->Keys[HashIndex * Self->MaxKeyLength],
                                   Key,
                                   KeyLength))
                {
                        Self->Values[HashIndex] = Value;
                        return(true);
                }
                HashIndex = (HashIndex + 1) % Self->Capacity;
        }
        while(HashIndex != StartHash);

        /* Couldn't find Key to update. */
        return(false);
}

gs_bool /* Wanted must be a NULL terminated string */
GSHashMapHasKey(gs_hash_map *Self, char *Wanted)
{
        unsigned int HashIndex = __GSHashMapComputeHash(Self, Wanted);
        char *Key = &Self->Keys[HashIndex * Self->MaxKeyLength];
        if(GSStringIsEqual(Wanted, Key, GSStringLength(Wanted)))
        {
                return(true);
        }

        unsigned int StartHash = HashIndex;
        HashIndex = (HashIndex + 1) % Self->Capacity;

        while(true)
        {
                if(HashIndex == StartHash) break;

                Key = &Self->Keys[HashIndex * Self->MaxKeyLength];
                if(GSStringIsEqual(Wanted, Key, GSStringLength(Wanted)))
                {
                        return(true);
                }
                HashIndex = (HashIndex + 1) % Self->Capacity;
        }

        return(false);
}

/*
  Input: Key as string
  Computation: Hash key value into an integer.
  Algorithm: Open-addressing hash. Easy to predict space usage.
             See: https://en.wikipedia.org/wiki/Open_addressing
  Key must be a NULL terminated string.
 */
gs_bool
GSHashMapSet(gs_hash_map *Self, char *Key, void *Value)
{
        unsigned int KeyLength = GSStringLength(Key);
        unsigned int HashIndex = __GSHashMapComputeHash(Self, Key);

        if(GSHashMapHasKey(Self, Key))
        {
                return __GSHashMapUpdate(Self, Key, Value);
        }

        /* We're not updating, so return false if we're at capacity. */
        if(Self->Count >= Self->Capacity) return(false);

        /* Add a brand-new key in. */
        if(Self->Keys[HashIndex * Self->MaxKeyLength] == GSNullChar)
        {
                GSStringCopy(Key, &Self->Keys[HashIndex * Self->MaxKeyLength], KeyLength);
                Self->Values[HashIndex] = Value;
                Self->Count++;
                return(true);
        }

        /* We have a collision! Find a free index. */
        unsigned int StartHash = HashIndex;
        HashIndex = (HashIndex + 1) % Self->Capacity;

        while(true)
        {
                if(HashIndex == StartHash) break;

                if(Self->Keys[HashIndex * Self->MaxKeyLength] == GSNullChar)
                {
                        GSStringCopy(Key, &Self->Keys[HashIndex * Self->MaxKeyLength], KeyLength);
                        Self->Values[HashIndex] = Value;
                        Self->Count++;
                        return(true);
                }
                HashIndex = (HashIndex + 1) % Self->Capacity;
        }

        /* Couldn't find any free space. */
        return(false);
}

gs_bool /* Memory must be large enough for the resized Hash. Memory _cannot_ overlap! */
GSHashMapGrow(gs_hash_map **Self, unsigned int NumEntries, void *New)
{
        gs_hash_map *Old = *Self;

        /* No point in making smaller... */
        if(NumEntries <= Old->Capacity) return(false);
        if(New == NULL) return(false);

        *Self = GSHashMapInit(New, Old->MaxKeyLength, NumEntries);
        for(int I=0; I<Old->Capacity; I++)
        {
                char *Key = &Old->Keys[I * Old->MaxKeyLength];
                char *Value = (char *)(Old->Values[I]);
                if(Key != NULL)
                {
                        gs_bool Success = GSHashMapSet(*Self, Key, Value);
                        if(!Success)
                                GSAbortWithMessage("This should have worked!\n");
                }
        }

        return(true);
}

void * /* Wanted must be a NULL terminated string */
GSHashMapGet(gs_hash_map *Self, char *Wanted)
{
        unsigned int HashIndex = __GSHashMapComputeHash(Self, Wanted);
        char *Key = &Self->Keys[HashIndex * Self->MaxKeyLength];
        if(GSStringIsEqual(Wanted, Key, GSStringLength(Key)))
        {
                void *Result = Self->Values[HashIndex];
                return(Result);
        }

        unsigned int StartHash = HashIndex;
        HashIndex = (HashIndex + 1) % Self->Capacity;

        while(true)
        {
                if(HashIndex == StartHash) break;

                Key = &Self->Keys[HashIndex * Self->MaxKeyLength];
                if(GSStringIsEqual(Wanted, Key, GSStringLength(Key)))
                {
                        void *Result = Self->Values[HashIndex];
                        return(Result);
                }
                HashIndex = (HashIndex + 1) % Self->Capacity;
        }

        return(NULL);
}

void * /* Wanted must be a NULL terminated string */
GSHashMapDelete(gs_hash_map *Self, char *Wanted)
{
        unsigned int HashIndex = __GSHashMapComputeHash(Self, Wanted);
        char *Key = &Self->Keys[HashIndex * Self->MaxKeyLength];
        if(GSStringIsEqual(Wanted, Key, GSStringLength(Key)))
        {
                void *Result = Self->Values[HashIndex];
                Self->Values[HashIndex] = NULL;
                Self->Keys[HashIndex * Self->MaxKeyLength] = GSNullChar;
                Self->Count--;
                return(Result);
        }

        unsigned int StartHash = HashIndex;
        HashIndex = (HashIndex + 1) % Self->Capacity;

        while(true)
        {
                if(HashIndex == StartHash) break;

                Key = &Self->Keys[HashIndex * Self->MaxKeyLength];
                if(GSStringIsEqual(Wanted, Key, GSStringLength(Key)))
                {
                        void *Result = Self->Values[HashIndex];
                        Self->Values[HashIndex] = NULL;
                        Self->Keys[HashIndex * Self->MaxKeyLength] = GSNullChar;
                        Self->Count--;
                        return(Result);
                }
                HashIndex = (HashIndex + 1) % Self->Capacity;
        }

        return(NULL);
}

/******************************************************************************
 * Arg Parsing
 ******************************************************************************/

typedef struct gs_args
{
        int Count;
        char **Args;
} gs_args;

unsigned int
GSArgsAllocSize(void)
{
        unsigned int Result = sizeof(gs_args);
        return(Result);
}

gs_args *
GSArgsInit(void *Memory, int ArgCount, char **Args)
{
        gs_args *Self = (gs_args *)Memory;
        Self->Count = ArgCount;
        Self->Args = Args;
        return(Self);
}

char *
GSArgsProgramName(gs_args *Self)
{
        char *ProgramName = Self->Args[0];
        char *Result = basename(ProgramName);
        return(Result);
}

gs_bool
GSArgsIsPresent(gs_args *Args, char *Wanted)
{
        int StringLength = GSStringLength(Wanted);
        for(int I=0; I<Args->Count; I++)
        {
                if(GSStringIsEqual(Wanted, Args->Args[I], StringLength))
                {
                        return(true);
                }
        }
        return(false);
}

int /* Returns -1 if Arg not found. */
GSArgsFind(gs_args *Args, char *Wanted)
{
        int StringLength = GSStringLength(Wanted);
        for(int I=0; I<Args->Count; I++)
        {
                if(GSStringIsEqual(Wanted, Args->Args[I], StringLength))
                {
                        return(I);
                }
        }
        return(-1);
}

char * /* Returns NULL if Index is invalid. */
GSArgsAtIndex(gs_args *Args, int Index)
{
        if((Index < 0) ||
           (Index > (Args->Count - 1)))
                return(NULL);
        else
                return(Args->Args[Index]);
}

char * /* Returns NULL if Marker is not found or no trailing arg. */
GSArgsAfter(gs_args *Args, char *Marker)
{
        int Index = GSArgsFind(Args, Marker);
        if(Index < 0) return(NULL);

        char *Arg = GSArgsAtIndex(Args, Index + 1);
        return(Arg);
}

gs_bool
GSArgsHelpWanted(gs_args *Args)
{
        if(GSArgsIsPresent(Args, "-h") ||
           GSArgsIsPresent(Args, "--help"))
                return(true);
        else
                return(false);
}

/******************************************************************************
 * Byte streams / Buffers / File IO
 ******************************************************************************/

typedef struct gs_buffer
{
        char *Start;
        char *Cursor;
        size_t Capacity;
        size_t Length;
        char *SavedCursor;
} gs_buffer;

gs_buffer *
GSBufferInit(gs_buffer *Buffer, char *Start, size_t Size)
{
        Buffer->Start = Start;
        Buffer->Cursor = Start;
        Buffer->Length = 0;
        Buffer->Capacity = Size;
        Buffer->SavedCursor = NULL;
        return(Buffer);
}

gs_bool
GSBufferIsEOF(gs_buffer *Buffer)
{
        int Size = Buffer->Cursor - Buffer->Start;
        gs_bool Result = Size >= Buffer->Length;
        return(Result);
}

void
GSBufferNextLine(gs_buffer *Buffer)
{
        while(true)
        {
                if(Buffer->Cursor[0] == '\n' ||
                   Buffer->Cursor[0] == '\0')
                {
                        break;
                }
                Buffer->Cursor++;
        }
        Buffer->Cursor++;
}

gs_bool
GSBufferSaveCursor(gs_buffer *Buffer)
{
        Buffer->SavedCursor = Buffer->Cursor;
        return(true);
}

gs_bool
GSBufferRestoreCursor(gs_buffer *Buffer)
{
        if(Buffer->SavedCursor == NULL) return(false);

        Buffer->Cursor = Buffer->SavedCursor;
        Buffer->SavedCursor = NULL;
        return(true);
}

size_t
GSFileSize(char *FileName)
{
        size_t FileSize = 0;
        FILE *File = fopen(FileName, "r");
        if(File != NULL)
        {
                fseek(File, 0, SEEK_END);
                FileSize = ftell(File);
                fclose(File);
        }
        return(FileSize);
}

gs_bool
GSFileCopyToBuffer(char *FileName, gs_buffer *Buffer)
{
        FILE *File = fopen(FileName, "r");
        if(File == NULL) return(false);

        fseek(File, 0, SEEK_END);
        size_t FileSize = ftell(File);
        int Remaining = (Buffer->Start + Buffer->Capacity) - Buffer->Cursor;
        if(FileSize > Remaining) return(false);

        fseek(File, 0, SEEK_SET);
        fread(Buffer->Cursor, 1, FileSize, File);
        Buffer->Length += FileSize;
        Buffer->Cursor += FileSize;
        *(Buffer->Cursor) = '\0';

        return(true);
}

#endif /* GS_VERSION */
