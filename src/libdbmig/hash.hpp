/*
    dbmig - Database schema migration tool
    Copyright (C) 2012-2014  Adam Szmigin (adam.szmigin@xsco.net)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DBMIG_HASH_INCLUDED
#define DBMIG_HASH_INCLUDED

#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

namespace dbmig {

///
/// The hash class wraps a Crypto++ hash.
///
template <typename T>
class hash
{
public:
    
    void update(const std::string &str)
    {
        cryptopp_hash_.Update(
            reinterpret_cast<const byte *>(str.c_str()),
            str.length());
    }
    
    void finalise()
    {
        cryptopp_hash_.Final(digest_);
    }
    
    void hex_encode(std::string &output) const
    {
        CryptoPP::HexEncoder encoder{NULL, false}; // Prefer lower-case
        encoder.Attach(new CryptoPP::StringSink(output));
        encoder.Put(digest_, sizeof(digest_));
        encoder.MessageEnd();
    }
    
private:

    T cryptopp_hash_;
    byte digest_[T::DIGESTSIZE];
};

typedef hash<CryptoPP::SHA256> sha256_hash;


} // dbmig

#endif // DBMIG_TIME_INCLUDED

