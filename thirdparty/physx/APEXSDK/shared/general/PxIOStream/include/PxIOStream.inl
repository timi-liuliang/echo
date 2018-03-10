/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


/*!
\file
\brief PxIOStream inline implementation
*/

PX_INLINE PxIOStream& PxIOStream::operator<<(bool v)
{
	if ( mBinary )
	{
		mStream.storeByte((physx::PxU8)v);
	}
	else
	{
		char scratch[6];
		storeString( physx::PxAsc::valueToStr(v, scratch, 6) );
	}
	return *this;
};


PX_INLINE PxIOStream& PxIOStream::operator<<(char c)
{
	mStream.storeByte((physx::PxU8)c);
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxU8 c)
{
	if ( mBinary )
	{
		mStream.storeByte((physx::PxU8)c);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(c, scratch, physx::PxAsc::IntStrLen) );
	}

	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxI8 c)
{
	if ( mBinary )
	{
		mStream.storeByte((physx::PxU8)c);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(c, scratch, physx::PxAsc::IntStrLen) );
	}

	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(const char *c)
{
	if ( mBinary )
	{
		c = c ? c : ""; // it it is a null pointer, assign it to an empty string.
		physx::PxU32 len = (physx::PxU32)strlen(c);
		PX_ASSERT( len < (MAX_STREAM_STRING-1));
		if ( len > (MAX_STREAM_STRING-1) )
		{
			len = MAX_STREAM_STRING-1;
		}
		mStream.storeDword(len);
		if ( len )
			mStream.write(c,len);
	}
	else
	{
		storeString(c);
	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxU64 v)
{
	if ( mBinary )
	{
		mStream.storeDouble( (physx::PxF64) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxI64 v)
{
	if ( mBinary )
	{
		mStream.storeDouble( (physx::PxF64) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxF64 v)
{
	if ( mBinary )
	{
		mStream.storeDouble( (physx::PxF64) v );
	}
	else
	{
		char scratch[physx::PxAsc::PxF64StrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::PxF64StrLen) );
	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxF32 v)
{
	if ( mBinary )
	{
		mStream.storeFloat(v);
	}
	else
	{
		char scratch[physx::PxAsc::PxF32StrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::PxF32StrLen) );

	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxU32 v)
{
	if ( mBinary )
	{
		mStream.storeDword(v);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxI32 v)
{
	if ( mBinary )
	{
		mStream.storeDword( (physx::PxU32) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxU16 v)
{
	if ( mBinary )
	{
		mStream.storeWord(v);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
};

PX_INLINE PxIOStream& PxIOStream::operator<<(physx::PxI16 v)
{
	if ( mBinary )
	{
		mStream.storeWord( (physx::PxU16) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
};


PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxU32 &v)
{
	if ( mBinary )
	{
		v = mStream.readDword();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(char &v)
{
	if ( mBinary )
	{
		v = (char)mStream.readByte();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxU8 &v)
{
	if ( mBinary )
	{
		v = (physx::PxU8)mStream.readByte();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxI8 &v)
{
	if ( mBinary )
	{
		v = (physx::PxI8)mStream.readByte();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxI64 &v)
{
	if ( mBinary )
	{
		v = (physx::PxI64)mStream.readDword();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxU64 &v)
{
	if ( mBinary )
	{
		v = (physx::PxU64)mStream.readDouble();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxF64 &v)
{
	if ( mBinary )
	{
		v = mStream.readDouble();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxF32 &v)
{
	if ( mBinary )
	{
		v = mStream.readFloat();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxI32 &v)
{
	if ( mBinary )
	{
		v = (physx::PxI32)mStream.readDword();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxU16 &v)
{
	if ( mBinary )
	{
		v = mStream.readWord();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxI16 &v)
{
	if ( mBinary )
	{
		v = (physx::PxI16)mStream.readWord();
	}
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(bool &v)
{
	physx::PxI8 iv;
	iv = (physx::PxI8)mStream.readByte();
	v = iv ? true : false;
	return *this;
};

#define NX_IOSTREAM_COMMA_SEPARATOR if(!mBinary) *this << ' ';

PX_INLINE PxIOStream& PxIOStream::operator<<(const physx::PxVec3 &v)
{
	*this << v.x; 
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.y;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.z;
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator<<(const physx::PxQuat &v)
{
	*this << v.x;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.y;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.z;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.w;
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator<<(const physx::PxMat33Legacy &v)
{
	physx::PxF32 mat[9];
	v.getColumnMajor(mat);
	*this << mat[0];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[1];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[2];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[3];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[4];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[5];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[6];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[7];
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << mat[8];
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator<<(const physx::PxMat34Legacy &v)
{
	*this << v.M;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.t;
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator<<(const physx::PxBounds3 &v)
{
	*this << v.minimum;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.maximum;
	return *this;
}



PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxVec3 &v)
{
	*this >> v.x;
	*this >> v.y;
	*this >> v.z;
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxQuat &v)
{
	*this>>v.x;
	*this>>v.y;
	*this>>v.z;
	*this>>v.w;
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxMat33Legacy &v)
{
	physx::PxF32 mat[9];
	*this >> mat[0];
	*this >> mat[1];
	*this >> mat[2];
	*this >> mat[3];
	*this >> mat[4];
	*this >> mat[5];
	*this >> mat[6];
	*this >> mat[7];
	*this >> mat[8];
	v.setColumnMajor(mat);
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxMat34Legacy &v)
{
	*this >> v.M;
	*this >> v.t;
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(physx::PxBounds3 &v)
{
	*this >> v.minimum;
	*this >> v.maximum;
	return *this;
}

PX_INLINE PxIOStream& PxIOStream::operator>>(const char *&str)
{
	str = NULL; // by default no string streamed...
	if ( mBinary )
	{
		physx::PxU32 len=0;
		*this >> len;
		PX_ASSERT( len < (MAX_STREAM_STRING-1) );
		if ( len < (MAX_STREAM_STRING-1) )
		{
			mStream.read(mReadString,len);
			mReadString[len] = 0;
			str = mReadString;
		}
	}
	return *this;
}


PX_INLINE void  PxIOStream::storeString(const char *c,bool zeroTerminate)
{
	while ( *c )
	{
		mStream.storeByte((physx::PxU8)*c);
		c++;
	}
	if ( zeroTerminate )
	{
		mStream.storeByte(0);
	}
}
