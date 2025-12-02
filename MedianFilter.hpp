/*
   MedianFilter.cpp - Median Filter for the Arduino platform.
   Copyright (c) 2013 Phillip Schmidt.  All right reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
    A median filter object is created by by passing the desired filter window size on object creation.
   The window size should be an odd number between 3 and 255.

   New data is added to the median filter by passing the data through the in() function.  The new medial value is returned.
   The new data will over-write the oldest data point, then be shifted in the array to place it in the correct location.

   The current median value is returned by the out() function for situations where the result is desired without passing in new data.

   !!! All data must be type INT.  !!!

   Window Size / avg processing time [us]
   5  / 22
   7  / 30
   9  / 40
   11 / 49
   21 / 99

*/

#include "MedianFilter.h"

template <typename T, typename Sum>
MedianFilter<T, Sum>::MedianFilter(int size, T seed)
{
   medFilterWin    = constrain(size, 3, 255); // number of samples in sliding median filter window - usually odd #
   medDataPointer  = medFilterWin >> 1;           // mid point of window
   data            = (T*) calloc (medFilterWin, sizeof(T)); // array for data
   sizeMap         = (uint8_t*) calloc (medFilterWin, sizeof(uint8_t)); // array for locations of data in sorted list
   locationMap     = (uint8_t*) calloc (medFilterWin, sizeof(uint8_t)); // array for locations of history data in map list
   oldestDataPoint = medDataPointer;      // oldest data point location in data array
   totalSum        = medFilterWin * ((Sum) seed);         // total of all values

   for(uint8_t i = 0; i < medFilterWin; i++) // initialize the arrays
   {
      sizeMap[i]     = i;      // start map with straight run
      locationMap[i] = i;      // start map with straight run
      data[i]        = seed;   // populate with seed value
   }
}

template <typename T, typename Sum>
MedianFilter<T, Sum>::MedianFilter(const MedianFilter<T, Sum> &other) :
   medFilterWin { other.medFilterWin },
   medDataPointer { other.medDataPointer },
   oldestDataPoint { other.oldestDataPoint },
   totalSum { other.totalSum } {
   data            = (T*) calloc (medFilterWin, sizeof(T)); // array for data
   sizeMap         = (uint8_t*) calloc (medFilterWin, sizeof(uint8_t)); // array for locations of data in sorted list
   locationMap     = (uint8_t*) calloc (medFilterWin, sizeof(uint8_t)); // array for locations of history data in map list
   memcpy(data, other.data, medFilterWin * sizeof(T));
   memcpy(sizeMap, other.sizeMap, medFilterWin * sizeof(uint8_t));
   memcpy(locationMap, other.locationMap, medFilterWin * sizeof(uint8_t));
}

template <typename T, typename Sum>
MedianFilter<T, Sum>& MedianFilter<T, Sum>::operator=(const MedianFilter<T, Sum>& other) {
   medFilterWin = other.medFilterWin;
   medDataPointer = other.medDataPointer;
   oldestDataPoint = other.oldestDataPoint;
   totalSum = other.totalSum;
   free(data);
   free(sizeMap);
   free(locationMap);
   data            = (T*) calloc (medFilterWin, sizeof(T)); // array for data
   sizeMap         = (uint8_t*) calloc (medFilterWin, sizeof(uint8_t)); // array for locations of data in sorted list
   locationMap     = (uint8_t*) calloc (medFilterWin, sizeof(uint8_t)); // array for locations of history data in map list
   memcpy(data, other.data, medFilterWin * sizeof(T));
   memcpy(sizeMap, other.sizeMap, medFilterWin * sizeof(uint8_t));
   memcpy(locationMap, other.locationMap, medFilterWin * sizeof(uint8_t));

   return *this;
}

template <typename T, typename Sum>
MedianFilter<T, Sum>::MedianFilter(MedianFilter<T, Sum> &&other) :
   medFilterWin { other.medFilterWin },
   medDataPointer { other.medDataPointer },
   data { other.data },
   sizeMap { other.sizeMap },
   locationMap { other.locationMap },
   oldestDataPoint { other.oldestDataPoint },
   totalSum { other.totalSum } {
   other.data = nullptr;
   other.sizeMap = nullptr;
   other.locationMap = nullptr;
}

template <typename T, typename Sum>
MedianFilter<T, Sum>& MedianFilter<T, Sum>::operator=(MedianFilter<T, Sum>&& other) {
   medFilterWin = other.medFilterWin;
   medDataPointer = other.medDataPointer;
   oldestDataPoint = other.oldestDataPoint;
   totalSum = other.totalSum;
   free(data);
   free(sizeMap);
   free(locationMap);
   data = other.data;
   sizeMap = other.sizeMap;
   locationMap = other.locationMap;
   other.data = nullptr;
   other.sizeMap = nullptr;
   other.locationMap = nullptr;
   return *this;
}

template <typename T, typename Sum>
MedianFilter<T, Sum>::~MedianFilter()
{
  // Free up the used memory when the object is destroyed
  free(data);
  free(sizeMap);
  free(locationMap);
}

template <typename T, typename Sum>
T MedianFilter<T, Sum>::in(const T & value)
{
   // sort sizeMap
   // small vaues on the left (-)
   // larger values on the right (+)

   bool dataMoved = false;
   const uint8_t rightEdge = medFilterWin - 1;  // adjusted for zero indexed array

   totalSum += ((Sum) value) - data[oldestDataPoint];  // add new value and remove oldest value

   data[oldestDataPoint] = value;  // store new data in location of oldest data in ring buffer

   // SORT LEFT (-) <======(n) (+)
   if(locationMap[oldestDataPoint] > 0) // don't check left neighbours if at the extreme left
   {
      for(uint8_t i = locationMap[oldestDataPoint]; i > 0; i--)   //index through left adjacent data
      {
         uint8_t n = i - 1;   // neighbour location

         if(data[oldestDataPoint] < data[sizeMap[n]]) // find insertion point, move old data into position
         {
            sizeMap[i] = sizeMap[n];   // move existing data right so the new data can go left
            locationMap[sizeMap[n]]++;

            sizeMap[n] = oldestDataPoint; // assign new data to neighbor position
            locationMap[oldestDataPoint]--;

            dataMoved = true;
         }
         else
         {
            break; // stop checking once a smaller value is found on the left
         }
      }
   }

   // SORT RIGHT (-) (n)======> (+)
   if(!dataMoved && locationMap[oldestDataPoint] < rightEdge) // don't check right if at right border, or the data has already moved
   {
      for(int i = locationMap[oldestDataPoint]; i < rightEdge; i++)   //index through left adjacent data
      {
         int n = i + 1;   // neighbour location

         if(data[oldestDataPoint] > data[sizeMap[n]]) // find insertion point, move old data into position
         {
            sizeMap[i] = sizeMap[n];   // move existing data left so the new data can go right
            locationMap[sizeMap[n]]--;

            sizeMap[n] = oldestDataPoint; // assign new data to neighbor position
            locationMap[oldestDataPoint]++;
         }
         else
         {
            break; // stop checking once a smaller value is found on the right
         }
      }
   }
   oldestDataPoint++;       // increment and wrap
   if(oldestDataPoint == medFilterWin) oldestDataPoint = 0;

   return data[sizeMap[medDataPointer]];
}

template <typename T, typename Sum>
T MedianFilter<T, Sum>::out() const // return the value of the median data sample
{
   return  data[sizeMap[medDataPointer]];
}

template <typename T, typename Sum>
T MedianFilter<T, Sum>::getMin() const
{
   return data[sizeMap[ 0 ]];
}

template <typename T, typename Sum>
T MedianFilter<T, Sum>::getMax() const
{
   return data[sizeMap[ medFilterWin - 1 ]];
}

template <typename T, typename Sum>
Sum MedianFilter<T, Sum>::getMean() const
{
   return totalSum / medFilterWin;
}

template <typename T, typename Sum>
Sum MedianFilter<T, Sum>::getStdDev() const // Arduino run time [us]: filterSize * 2 + 131
{
   Sum diffSquareSum = 0;
   Sum mean = getMean();

   for( int i = 0; i < medFilterWin; i++ )
   {
      Sum diff = data[i] - mean;
      diffSquareSum += diff * diff;
   }

   return Sum( sqrt( ((double)(diffSquareSum / (medFilterWin - 1.0))) + 0.5 ) );
}


// *** debug fuctions ***
/*
void MedianFilter::printData() // display sorting data for debugging
{
   for(int i=0; i<medFilterWin; i++)
   {
      Serial.print(data[i]);
      Serial.print("\t");
   }
   Serial.println("Data in ring buffer");
}

void MedianFilter::printSizeMap()
{
   for(int i=0; i<medFilterWin; i++)
   {
      Serial.print(sizeMap[i]);
      Serial.print("\t");
   }
   Serial.println("Size Map, data sorted by size");
}

void MedianFilter::printLocationMap()
{
   for(int i=0; i<medFilterWin; i++)
   {
      Serial.print(locationMap[i]);
      Serial.print("\t");
   }
   Serial.println("Location Map, size data sorted by age");
}

void MedianFilter::printSortedData() // display data for debugging
{
   for(int i=0; i<medFilterWin; i++)
   {
      Serial.print(data[sizeMap[i]]);
      Serial.print("\t");
   }
   Serial.println("Data sorted by size");
}
*/
