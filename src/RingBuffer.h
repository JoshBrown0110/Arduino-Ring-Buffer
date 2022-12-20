#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "Arduino.h"

#define CLEAR_BITMASK(x, y)    (x &= (~y))
#define SET_BITMASK(x, y)      (x |= (y))
#define CHECK_BITMASK(x, y)    (x & (y))

#define FLAG_FULL    0x01
#define FLAG_EMPTY   0x02

/**
 * A general-purpose RingBuffer. Through the use of templates, allows specialized types. Also specializes said template
 * based on the size of the variable. Returns the raw value if the amount of bytes used by the type is less than or 
 * equal to that of a pointer.
 */
template<typename T, uint8_t value = (sizeof(T) <= 2)>
class RingBuffer{
   public:
      /**
       * Construct a new Ring Buffer object. Uses the passed array as the ring buffer. Avoid using the array outside of
       * this object to prevent the indexes from desyncing.
       */
      RingBuffer(T *arr, uint8_t elements){
         ringBuff *b = &buff;    //On AVR Architecture: Initializing Z-Pointer to our RingBuffer struct.

         b->addr = arr;          //Set our array address to the passed address.
         b->arrLen = elements;   //Set the number of elements we have in the array.
      };

      /**
       * Writes a new value to the RingBuffer. Takes a pointer to a data location, of type T, and stores it in the
       * array. When the buffer is full, it will override the oldest value by default. 
       * 
       * To change the way that a full buffer is handled, the user is expected to use their own function to determine
       * if this method should be called or not.
       */
      inline uint8_t write(T const *data){
         ringBuff *b = &buff;                //On AVR Architecture: Initializing Z-Pointer to our RingBuffer struct.
             
         b->addr[b->writePos++] = *data;     //Takes the data stored at the passed memory location, and stores it in the ring buffer.
         return (b->writePos %= b->arrLen);  //Limits the bounds of the internal index, and returns the next index value.
      };

      /**
       * Writes a new value to the RingBuffer, with protection. Performs data storage identically to the write()
       * method, except this method performs checks on indices to prevent the oldest bit of data from being overwritten.
       * 
       * When the buffer is full and this method is called, will return the current write index instead of the next.
       * 
       * May not be the most optimized method, or most desired behavior. 
       */
      uint8_t p_write(T const *data){
         ringBuff *b = &buff;
         uint8_t ret;

         if(__builtin_expect(CHECK_BITMASK(b->flags, FLAG_FULL), 0)){
            ret = b->writePos - 1;
            if(__builtin_expect(ret > b->arrLen, 0)){
               ret = b->arrLen;
            }
         }else{
            b->addr[b->writePos++] = *data;
            ret = (b->writePos %= b->arrLen);

            CLEAR_BITMASK(b->flags, FLAG_EMPTY);

            if(__builtin_expect(ret == b->readPos, 0)){
               SET_BITMASK(b->flags, FLAG_FULL);
            }
         }

         return ret;
      }

      /**
       * Read the next value from the ring buffer. Returns a pointer to the next item in this array. If the buffer is
       * empty, it will return the oldest value in the array next. 
       * 
       * To change the way an empty buffer is handled, the user is expected to use their own function to determine if
       * this method should be called or not. 
       */
      inline T* read(){
         ringBuff *b = &buff;                //On AVR Architecture: Initializing Z-Pointer to our RingBuffer struct.

         T* item = &b->addr[b->readPos++];   //Grabs the address value of the item at read index, then increase read index by 1.
         b->readPos %= b->arrLen;            //Ensure the read index is within bounds.
         return item;                        //Returns the address that we previously grabbed
      };
   
      /**
       * Read the next value from the ring buffer, with protection. Reads identically to the read() method, except this
       * method performs checks on indices to prevent reading past the write index. 
       * 
       * When the buffer is empty and this method is called, will return the previous item returned. This can prompt
       * the user to check if the buffer is full by looking for duplicate return values.
       * 
       * May not be the most optimized method, or most desired behavior.
       */
      T* p_read(){
         ringBuff *b = &buff;
         uint8_t index;
         T* item;

         if(__builtin_expect(CHECK_BITMASK(b->flags, FLAG_EMPTY), 0)){
            index = b->readPos - 1;
            if(__builtin_expect(index > b->arrLen, 0)){
               index = b->arrLen - 1;
            }
         } else {
            item = &b->addr[index++];
            b->readPos = index % b->arrLen;

            CLEAR_BITMASK(b->flags, FLAG_FULL);

            if(__builtin_expect(index == b->writePos, 0)){
               SET_BITMASK(b->flags, FLAG_EMPTY);
            }
         }

         return item;
      }

      /**
       * A method that can be called to check if the ring buffer is full. This is faster than calling write() again.
       */
      inline uint8_t isFull(){
         return CHECK_BITMASK(buff->flags, FLAG_FULL);
      }

      /**
       * A method that can be called to check if the ring buffer is empty. This is faster than calling read() again.
       */
      inline uint8_t isEmpty(){
         return CHECK_BITMASK(buff->flags, FLAG_EMPTY);
      }

   private:
      struct ringBuff{
         T *addr;             //A pointer for an array of the type which will be stored.
         uint8_t arrLen;      //How many elements long this RingBuffer will be.
         uint8_t readPos;     //The index used for reading from the buffer.
         uint8_t writePos;    //The index used for writing from the buffer.
         uint8_t flags;       //A series of flags used to store information and configuration for the buffer.
      };
      ringBuff buff;          //The RingBuffer struct which contains all global variables. 
};


/**
 * A specialized RingBuffer. Optimized for types that are less than or equal to the amount of bytes that a pointer is
 * stored in. Directly returns the value instead of the pointer to said value. Increases speed for variables less than
 * the pointer and removes pointer syntax for variables that are equal.
 * 
 * To bypass this specialization, set the second argument for your template to be 0, enter a single argument otherwise.
 */
template<typename T>
class RingBuffer<T, 1>{
   public:
      /**
       * Construct a new Ring Buffer object. Uses the passed array as the ring buffer. Avoid using the array outside of
       * this object to prevent the indexes from desyncing.
       */
      RingBuffer(T *arr, uint8_t elements){
         ringBuff *b = &buff;             //On AVR Architecture: Initializing Z-Pointer to our RingBuffer struct.

         b->addr = arr;                   //Set our array address to the passed address.
         b->arrLen = elements;            //Set the number of elements we have in the array.
      };

      /**
       * Writes a new value to the RingBuffer. Takes the data directly and stores a copy of that data in the array.
       * When the buffer is full, it will override the oldest value by default.
       * 
       * To change the way that a full buffer is handled, the user is expected to use their own function to determine
       * if this method should be called or not.
       */
      inline uint8_t write(T data){
         ringBuff *b = &buff;                   //On AVR Architecture: Initializing Z-Pointer to our RingBuffer struct.

         b->addr[b->writePos++] = data;         //Sets the value of the buffer at index W_POS to the passed data value.
         return (b->writePos %= b->arrLen);     //Returns the next usable index of the buffer.
      };

      /**
       * Writes a new value to the RingBuffer, with protection. Performs data storage identically to the write()
       * method, except this method performs checks on indices to prevent the oldest bit of data from being overwritten.
       * 
       * When the buffer is full and this method is called, will return the current write index instead of the next.
       * 
       * May not be the most optimized method, or most desired behavior. 
       */
      uint8_t p_write(T const data){
         ringBuff *b = &buff;
         uint8_t ret;

         if(__builtin_expect(CHECK_BITMASK(b->flags, FLAG_FULL), 0)){
            ret = b->writePos - 1;
            if(__builtin_expect(ret > b->arrLen, 0)){
               ret = b->arrLen;
            }
         }else{
            b->addr[b->writePos++] = data;
            ret = (b->writePos %= b->arrLen);

            CLEAR_BITMASK(b->flags, FLAG_EMPTY);

            if(__builtin_expect(ret == b->readPos, 0)){
               SET_BITMASK(b->flags, FLAG_FULL);
            }
         }

         return ret;
      }

      /**
       * Read the next value from the ring buffer. Returns the data at the current read point in the ring buffer. If
       * the buffer is empty, it will return the oldest value in the array.
       * 
       * To change the way an empty buffer is handled, the user is expected to use their own function to determine if
       * this method should be called or not. 
       */
      inline T read(){
         ringBuff *b = &buff;             //On AVR Architecture: Initializing Z-Pointer to our RingBuffer struct.

         T item = b->addr[b->readPos++];  //Grabs the address value of the item at read index, then increase read index by 1.
         b->readPos %= b->arrLen;         //Ensure the read index is within bounds.
         return item;                     //Returns the address that we previously grabbed
      };

      /**
       * Read the next value from the ring buffer, with protection. Reads identically to the read() method, except this
       * method performs checks on indices to prevent reading past the write index. 
       * 
       * When the buffer is empty and this method is called, will return the previous item returned. This can prompt
       * the user to check if the buffer is full by looking for duplicate return values.
       * 
       * May not be the most optimized method, or most desired behavior.
       */
      T* p_read(){
         ringBuff *b = &buff;
         uint8_t index;
         T item;

         if(__builtin_expect(CHECK_BITMASK(b->flags, FLAG_EMPTY), 0)){
            index = b->readPos - 1;
            if(__builtin_expect(index > b->arrLen, 0)){
               index = b->arrLen - 1;
            }
         } else {
            item = b->addr[index++];
            b->readPos = index % b->arrLen;

            CLEAR_BITMASK(b->flags, FLAG_FULL);

            if(__builtin_expect(index == b->writePos, 0)){
               SET_BITMASK(b->flags, FLAG_EMPTY);
            }
         }

         return item;
      }

      /**
       * A method that can be called to check if the ring buffer is full. This is faster than calling write() again.
       */
      inline uint8_t isFull(){
         return CHECK_BITMASK(buff->flags, FLAG_FULL);
      }

      /**
       * A method that can be called to check if the ring buffer is empty. This is faster than calling read() again.
       */
      inline uint8_t isEmpty(){
         return CHECK_BITMASK(buff->flags, FLAG_EMPTY);
      }

   private:
      struct ringBuff{
         T *addr;
         uint8_t arrLen;
         uint8_t readPos;
         uint8_t writePos;
         uint8_t flags;
      };
      ringBuff buff;
};

#endif