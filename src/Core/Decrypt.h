#pragma once
#include <cstdint>
#include <immintrin.h>

namespace ARC {
    namespace Decrypt {
        struct gworld_key { char pad[32]; uint64_t pTable; char pad2[88]; __m128i mKey; };
        struct gworld_table { uint64_t base[2]{ 0 }; uint64_t pTable[2]{ 0 }; __m128i xmm_1[2]{ 0 }; };

        struct camera_key { uint64_t pTable; char pad[72]; __m128i mKey; };
        struct camera_table { uint64_t pTable[2]{ 0 }; __m128i xmm_1[2]{ 0 }; };

        
        uint64_t GetGWorld(uint32_t offset);
        uint64_t GetGameInstance(uint64_t world);
        uint64_t GetPlayerCameraManager(uint64_t controller);
        void DecodeString(uint16_t* s, int32_t maxLenChars);

        namespace methods {
            namespace world {
                bool attack(const uint32_t& offset, gworld_table& data);
                bool decrypt(const gworld_table& data, uint64_t& result);

                
                bool __a(const __m128i& input, uint64_t& result);
                bool __b(const __m128i& input, uint64_t& result);
                bool __c(const __m128i& input, uint64_t& result);
                bool __d(const __m128i& input, uint64_t& result);
                bool __e(const __m128i& input, uint64_t& result);
            }
            namespace game { bool decrypt(const uint64_t& world, uint64_t& result); }
            namespace camera {
                bool attack(const uint64_t& pc, camera_table& result);
                bool decrypt(const camera_table& data, uint64_t& result);
                bool __a(const __m128i& input, uint64_t& result);
                bool __b(const __m128i& input, uint64_t& result);
            }
        }
    }
}