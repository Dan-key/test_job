#include <iostream>

class packages{

    public:

    size_t size;
    size_t shift;
    char* beg;  // beginning 

    packages ( char* b = nullptr, size_t start = 0, size_t len = 0) : size(len), shift(start), beg(b){
        beg += shift;
    }

    //for making package out of other package
    packages ( packages& b , size_t start = 0, size_t len = 0){
        if( (b.size - start) < len){
            throw "Out of range of nested package";
        }

        this->size = len;
        this->beg = b.beg+start;
        shift = start;
    }


    
    packages(const packages& b ){
        this->beg= b.beg;
        this->shift = b.shift;
        this->size = b.size;
    }



    char& operator[](size_t i){
        if( i > this->size){
            throw "Out of range";
        }
        return *(beg+ i);
    }



    short getdata(int start, int len){
        if( start < len-2){
            throw "Start shoul be more then len";
        }

        short result = 0;
        int int_bytes_start = start/8; // first byte least significant
        int rem_bytes_start = (start)%8; 
        u_int32_t h = 0; // here we will save whole data
        int last_byte  = (start - len)/8; // last byte most significant 

        for (int i = last_byte; i <= int_bytes_start; ++i){
            u_int32_t value_in_mem= static_cast<u_int32_t>( static_cast<unsigned char>(*(this->beg+i)));
            value_in_mem = value_in_mem<<((int_bytes_start - i)*8);
            h = h+ value_in_mem; // put memory from this->beg into val
        }

        h = h >> (8 -  rem_bytes_start - 1); // remove un_used bits from right
        u_int32_t remove_right_part  = (1<<len)-1;
        h = h & remove_right_part; // removing right part
        bool sign = h>>(len-1) == 1; // if true then negative

        if ( sign){
            h = h - (1<<(len-1) );  // removing sign bit
            result = -h;
        }else{
            result = h;
        }

        return result;
    }




    void putdata( int start, int len, short value){
        if( abs(value) >= 1<<len){
            throw "Not enough bits";
        }
        if( start < len-2){
                throw "Start shoul be more then len";
        }

        int int_bytes_start = start/8; // first byte least significant part
        int rem_bytes_start = (start)%8;  
        int last_byte  = (start - len+1)/8;// last byte most significant 
        bool sign = value < 0; // if true then negative
        value  = abs(value); 

        if( int_bytes_start - last_byte == 0){ // if we work only in one byte
            unsigned char f = *(this->beg+last_byte); // saving in h first bits of byte before we put data
            int fisrt_bits = (start-len+1)%8; // number of first bits
            f = f>>(8-fisrt_bits); 
            f = f << ( 8 - fisrt_bits); // saving only first bits, everything else is zero
            unsigned char l = *(this->beg+last_byte);
            l = l <<( fisrt_bits + len);// last bits
            l = l >>( fisrt_bits + len); // saving only last bits, everything else is zero
            int shift_of_value = 8 - 1 - (start%8);
            if(sign)
                value+= 1<<(len-1); // put sign bit
            unsigned char value_char = value<<shift_of_value ;
            unsigned char final_byte = f;// here will be the final unsigned char we will put
            final_byte = final_byte| value_char;
            final_byte = final_byte|l;
            *(this->beg+last_byte) = static_cast<char>(final_byte);
        } 
        else if( int_bytes_start - last_byte == 1){ // takes to bytes to store
            // fisrt of all we need count how many bits value will take in first and last byte
            int num_of_bits1 = 8 - ((start - len+1)%8); // in first byte
            int num_of_bits2 = rem_bytes_start+1;// in last part
            unsigned char first_byte_char =  *(this->beg+last_byte);

            //clearing last bits where will be value
            first_byte_char= first_byte_char>>num_of_bits1;
            first_byte_char = first_byte_char << num_of_bits1;
            unsigned char value_byte_first_part; // here will  be store left part which we will assign to first byte
            unsigned char value_byte_last_part; //  here will  be store right part which we will assign to last byte


            if ( len > 8){
                u_int16_t value_2byte = static_cast<u_int16_t>(value); // here we  will store our value in 2 bytes
                if(sign)
                    value_2byte += 1<<(len-1); // put sign bit                                       removing unnecessary bits
                value_byte_first_part = static_cast<unsigned char>(((value_2byte) >> (num_of_bits2))&0b0000000011111111); // shift to the right position; 
                //                                                                                      removing unnecessary bits
                value_byte_last_part = static_cast<unsigned char>(((value_2byte) << ( 8 - num_of_bits2))&0b0000000011111111); // shift to the right position
            }else{
                unsigned char value_byte = static_cast<unsigned char>(value);
                if(sign)
                    value_byte += 1<<(len-1); // put sign bit
                value_byte_first_part = value_byte >> (num_of_bits2); // shift to the right position
                value_byte_last_part = value_byte << ( 8-num_of_bits2);
            }


            first_byte_char = first_byte_char| value_byte_first_part;
            *(this->beg + last_byte) =  static_cast<char>(first_byte_char); // storing right part
            unsigned char last_byte_char  = *(this->beg+int_bytes_start);

            //clearing bits for value part
            last_byte_char = last_byte_char<<num_of_bits2;
            last_byte_char = last_byte_char>>num_of_bits2;
            
            last_byte_char =last_byte_char|value_byte_last_part; //applying mask
            *(this->beg + int_bytes_start) =  static_cast<char>(last_byte_char); // storing left part
        }else{ // takes 3 bytes to store
            // fisrt of all we need count how many bits value will take in first and last byte
            int num_of_bits1 = 8 - ((start - len+1)%8); // in first byte
            int num_of_bits2 = rem_bytes_start+1; // last first byte
            unsigned char first_byte_char =  *(this->beg+last_byte); // store first byte from this->begfer to make mask
            unsigned char last_byte_char  = *(this->beg+int_bytes_start); // store first byte from this->begfer to make mask

            //clearing last bits where will be value
            first_byte_char= first_byte_char>>num_of_bits1;
            first_byte_char = first_byte_char << num_of_bits1;

            u_int32_t value_4byte = value;
            unsigned char value_byte_first_part; // here will  be store left value byte 
            unsigned char value_byte_middle_part; // here will  be store middle value byte 
            unsigned char value_byte_last_part; // here will  be store right value byte 

            if(sign)
                value_4byte += 1<<(len-1); // put sign bit                                              clearing all unnecessary bits                  
            value_byte_first_part = static_cast<unsigned char>((( value_4byte ) >> (num_of_bits2+8)) & 0b00000000'00000000'00000000'11111111); // shift to the right position
            value_byte_middle_part = static_cast<unsigned char>(((value_4byte ) >> (num_of_bits2)) & 0b00000000'00000000'00000000'11111111); // shift to the right position
            value_byte_last_part = static_cast<unsigned char>(((  value_4byte ) >> (8 - num_of_bits2-1)) & 0b00000000'00000000'00000000'11111111); // shift to the right position

            first_byte_char = first_byte_char | value_byte_first_part; // applying mask
            *(this->beg + last_byte) =  static_cast<char>(first_byte_char); // storing right part

            //clearing bits for value part
            last_byte_char = last_byte_char<<num_of_bits2;
            last_byte_char = last_byte_char>>num_of_bits2;

            last_byte_char =last_byte_char|value_byte_last_part; // applying mask
            *(this->beg + int_bytes_start) =  static_cast<char>(last_byte_char); // storing left part

            *(this->beg + last_byte + 1) = static_cast<char>(value_byte_middle_part); // storing middle part, middle part doesn't need mask, because all its bits will be taken
        }

    }

};