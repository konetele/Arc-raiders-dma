#include "Decrypt.h"
#include "Memory.h"
#include <intrin.h>

// Constants from defines.h / common.h
#define GWORLD_FRIEND 0x20
#define INSTANCE_A 0xAE932C0 
#define INSTANCE_FRIEND 0x30

// Virtual function addresses (RVA) - NEED TO UPDATE TO CURRENT PATCH
// These addresses change with every game update
#define WORLD_A 0x03211280  
#define WORLD_B 0x03243670 
#define WORLD_C 0x03208BD0  
#define WORLD_D 0x03208260  
#define WORLD_E 0x03217290

namespace ARC
{
    namespace Decrypt
    {
        static inline uint32_t rol32(uint32_t x, int r) {
            return (x << r) | (x >> (32 - r));
        }

        // Функция расшифровки строк (Theia String Decryption)
        void DecodeString(uint16_t* s, int32_t maxLenChars) {
            if (!s || maxLenChars <= 0) return;
            uint32_t v8 = 0;
            for (int32_t i = 0; i < maxLenChars; ++i) {
                uint16_t v6 = s[i];
                if (v6 == 0) break;
                uint32_t v9 = v8 + rol32(16777619u * v8 - 0x78BFF4D9, 16);
                v8 = 16777619u * v9;
                uint32_t key = (uint32_t)((uint8_t)v9);
                uint32_t x = (uint32_t)(((-109 * (int32_t)key) & 0x1F));
                uint32_t v10 = ((uint32_t)v6) ^ x;
                
                int32_t v11 = 0;
                if ((uint32_t)(v10 - 80) < 0x2F) v11 = -47;
                if ((uint32_t)(v10 - 33) < 0x2F) v11 = 47;
                
                int32_t v12 = v11 + (int32_t)v10 - 48;
                int32_t v13 = v11 + (int32_t)v10 - 53;
                int32_t v14 = (int32_t)v10 + v11;
                
                int32_t v15 = 5 * (v13 >= 5) - 5;
                if (v12 < 5) v15 = 5;
                
                int32_t v16 = v14 + v15;
                int32_t v17 = 0;
                if ((uint32_t)(v15 + v14 - 110) < 0xD) v17 = -13;
                if ((uint32_t)(v15 + v14 - 97) < 0xD) v17 = 13;
                
                s[i] = (uint16_t)(v16 + v17); 
            }
        }

        uint64_t GetGWorld(uint32_t offset)
        {
            gworld_table data;
            if (!methods::world::attack(offset, data)) return 0;
            
            uint64_t uworld = 0;
            if (!methods::world::decrypt(data, uworld)) return 0;
            
            return uworld;
        }

        uint64_t GetGameInstance(uint64_t world)
        {
            uint64_t result = 0;
            if (!methods::game::decrypt(world, result)) return 0;
            return result;
        }

        uint64_t GetPlayerCameraManager(uint64_t controller)
        {
            camera_table data;
            if (!methods::camera::attack(controller, data)) return 0;

            uint64_t pcm = 0;
            if (!methods::camera::decrypt(data, pcm)) return 0;
            return pcm;
        }

        // --- Реализация методов (сокращенно, основная логика из dumper) ---
        namespace methods
        {
            namespace world
            {
                bool attack(const uint32_t& offset, gworld_table& data)
                {
                    uint64_t mb = Memory::GetModuleBaseAddress();
                    if (!mb || !offset) return false;

                    uint64_t offset_1 = offset + mb;
                    uint64_t offset_2 = offset_1 - GWORLD_FRIEND;

                    // Логика расчета индексов (из Dumper)
                    uint32_t world_lower = (uint32_t)offset_1;
                    uint32_t world_upper = (uint32_t)(offset_1 >> 32);

                    uint32_t v0_inner1 = 16777619 * (world_lower >> 3) + 151568469;
                    uint32_t v0_inner2 = world_upper + 16777619 * _rotl(v0_inner1, 22) + 151568469;
                    uint32_t v0_inner3 = 16777619 * (v0_inner2 >> 3) + 151568469;
                    uint32_t v0 = 16777619 * (v0_inner3 >> 10) + 151568469;
                    uint32_t v1 = v0 ^ (v0 >> 16);

                    uint8_t index[2];
                    index[0] = ((uint8_t)v0 ^ (uint8_t)(v0 >> 16)) & 7;
                    index[1] = ((uint8_t)v1 + 1) & 7;

                    gworld_key key[2]{};
                    for (int i = 0; i < 2; i++)
                    {
                        uint64_t _base = offset_2 + (112 * index[i]);
                        // Читаем ключ через Memory::ReadBuffer
                        if (!Memory::ReadBuffer(_base, &key[i], sizeof(gworld_key)))
                            return false;

                        data.base[i] = _base;
                        data.pTable[i] = key[i].pTable;
                        data.xmm_1[i] = key[i].mKey;
                    }
                    return true;
                }

                // ... Сюда нужно перенести decrypt, __a, __b и т.д. из dumper/Decrypt.cpp
                
                // Пример адаптации одной функции:
                bool decrypt(const gworld_table& data, uint64_t& result)
                {

                   // Проверка VTable (WORLD_A, WORLD_B...) обязательна!
                   return false; 
                }
            }

            namespace game
            {
                bool decrypt(const uint64_t& world, uint64_t& result)
                {
                    uint64_t mb = Memory::GetModuleBaseAddress();
                    __m128i xmmword_AE932C0;
                    if (!Memory::ReadBuffer(mb + INSTANCE_A, &xmmword_AE932C0, 16)) return false;

                    __m128i xmmword_AE932F0;
                    if (!Memory::ReadBuffer((mb + INSTANCE_A) + INSTANCE_FRIEND, &xmmword_AE932F0, 16)) return false;

                    __m128i _v5 = Memory::Read<__m128i>(world + 0x2E0);
                    auto v5 = _mm_xor_si128(_mm_load_si128(&_v5), xmmword_AE932C0);
                    auto v6 = _mm_xor_si128(
                        _mm_shufflelo_epi16(_mm_or_si128(_mm_slli_epi64(v5, 0x2Bu), _mm_srli_epi64(v5, 0x15u)), 114),
                        xmmword_AE932F0).m128i_u64[0];

                    result = v6;
                    return true;
                }
            }

            namespace camera
            {
                bool attack(const uint64_t& pc, camera_table& result)
                {
                    uint64_t mb = Memory::GetModuleBaseAddress();
                    if (!mb || !pc)
                        return false;

                    const auto fn_offset = 96;
                    const auto key_offset = 1168;

                    // Calculate hash
                    const auto key1 = 16777619;
                    const auto key2 = 92299578;
                    auto v2 = 16777619 * ((uint32_t)(key1 * _rotl((uint32_t)((uint64_t)(pc + 1168) >> 32) + key1 * _rotl((uint32_t)(key1 * _rotl((uint32_t)(pc + 1168), 16) + key2), 26) + key2, 16) + key2) >> 6) + key2;
                    auto v3 = v2 ^ (v2 >> 16);

                    // Get indices
                    uint8_t index[2]{};
                    index[0] = ((uint8_t)v2 ^ (uint8_t)(v2 >> 16)) & 7u;
                    index[1] = (((uint8_t)v3 + 1)) & 7u;

                    // Structured reads
                    camera_key keys[2]{};
                    for (int i = 0; i < 2; i++)
                    {
                        uint64_t base = pc + key_offset + (fn_offset * index[i]);
                        if (!Memory::ReadBuffer(base, &keys[i], sizeof(camera_key)))
                            return false;

                        result.pTable[i] = keys[i].pTable;
                        result.xmm_1[i] = keys[i].mKey;
                    }

                    return result.pTable[0] != 0 && result.pTable[1] != 0;
                }

                bool decrypt(const camera_table& data, uint64_t& result)
                {
                    // Decrypt using first vtable/key
                    uint64_t result_1 = 0;
                    if (!__a(data.xmm_1[0], result_1))
                        return false;

                    // Decrypt using second vtable/key
                    uint64_t result_2 = 0;
                    if (!__b(data.xmm_1[1], result_2))
                        return false;

                    // Get xor key
                    uint64_t xor_key = 0x100000001B3ULL * _rotl64(
                        0x100000001B3ULL * _rotl64(result_1, 57) - 0x19B9E0D51BB6DB06ULL,
                        57
                    ) - 0x19B9E0D51BB6DB06ULL;

                    // Check if result_2 matches the key
                    if (result_2 == xor_key)
                    {
                        result = result_1;
                        return true;
                    }

                    // Return xor result
                    result = result_1 + (xor_key ^ result_2);
                    return true;
                }

                bool __a(const __m128i& input, uint64_t& result)
                {
                    uint64_t mb = Memory::GetModuleBaseAddress();

                    // Get mask and key
                    __m128i xor_key;
                    __m128i shuffle_mask;
                    if (!Memory::ReadBuffer(mb + 0xB1E8180, &xor_key, 0x10) ||
                        !Memory::ReadBuffer(mb + 0xB1E8170, &shuffle_mask, 0x10))
                        return false;

                    // Decrypt
                    auto a2 = reinterpret_cast<const __m128i*>(&input);
                    __m128i v2 = _mm_shuffle_epi8(_mm_load_si128(a2), shuffle_mask);
                    auto ret = _mm_xor_si128(_mm_or_si128(_mm_slli_epi32(v2, 0x12u), _mm_srli_epi32(v2, 0xEu)), xor_key);

                    // Store result
                    double buffer;
                    _mm_storel_pd(&buffer, _mm_castsi128_pd(ret));
                    result = *reinterpret_cast<uint64_t*>(&buffer);
                    return result > 0;
                }

                bool __b(const __m128i& input, uint64_t& result)
                {
                    uint64_t mb = Memory::GetModuleBaseAddress();

                    // Get mask and key
                    __m128i shuffle_mask;
                    __m128i xor_key;
                    if (!Memory::ReadBuffer(mb + 0xB1E8290, &shuffle_mask, 0x10) ||
                        !Memory::ReadBuffer(mb + 0xB1E82A0, &xor_key, 0x10))
                        return false;

                    // Decrypt
                    auto a2 = reinterpret_cast<const __m128i*>(&input);
                    __m128i si128 = _mm_load_si128(a2);
                    auto ret = _mm_xor_si128(
                        _mm_shuffle_epi8(
                            _mm_or_si128(_mm_slli_epi32(si128, 0x15u), _mm_srli_epi32(si128, 0xBu)),
                            shuffle_mask),
                        xor_key);

                    // Store result
                    double buffer;
                    _mm_storel_pd(&buffer, _mm_castsi128_pd(ret));
                    result = *reinterpret_cast<uint64_t*>(&buffer);
                    return result > 0;
                }
            }
        }
    }
}