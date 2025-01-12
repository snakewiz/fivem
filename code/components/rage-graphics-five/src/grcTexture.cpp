#include "StdInc.h"
#include "grcTexture.h"
#include "Hooking.h"

namespace rage
{
static grcTextureFactory** g_textureFactory;
static grcTexture** g_noneTexture;

grcTextureFactory* grcTextureFactory::getInstance()
{
	return *g_textureFactory;
}

grcTexture* grcTextureFactory::GetNoneTexture()
{
	return *g_noneTexture;
}
}

static HookFunction hookFunction([] ()
{
	char* location = hook::pattern("84 DB 48 0F 45 C2 48 89  05").count(1).get(0).get<char>(9);

	rage::g_textureFactory = (rage::grcTextureFactory**)(*(int32_t*)location + location + 4);

	location = hook::pattern("45 33 C0 48 8B CF FF 50  20 48 8D 15").count(1).get(0).get<char>(22);

	rage::g_noneTexture = (rage::grcTexture**)(*(int32_t*)location + location + 4);


	//rage::g_textureFactory = *hook::pattern("EB 02 33 C0 80 7C 24 04 00 74 05 A3").count(1).get(0).get<rage::grcTextureFactory**>(12);
	//rage::g_noneTexture = *hook::pattern("8B CE FF D2 A3 ? ? ? ? 83 47 20 FF 75 10").count(1).get(0).get<rage::grcTexture**>(5);
});

__declspec(dllexport) fwEvent<> OnD3DPostReset;

void ClearRenderTarget(bool a1, int value1, bool a2, float value2, bool a3, int value3)
{
	/*ClearRenderTargetDC dc;
	dc.value1 = value1;
	dc.value2 = value2;
	dc.value3 = value3;
	dc.a1 = a1;
	dc.a2 = a2;
	dc.a3 = a3;

	ClearRenderTargetDC__process(&dc);*/
}

bool __declspec(dllexport) rage::grcTexture::IsRenderSystemColorSwapped()
{
	// Five is only D3D10+, and seemingly didn't adopt the new D3D11 color order
	return true;
}

static hook::cdecl_stub<rage::grcResourceCache*()> _getResourceCache([]()
{
	return hook::get_call(hook::get_pattern("48 85 FF 74 0B 48 8B D7 48 8B C8 E8 ? ? ? ? 48 83 63 28 00", -5));
});

static hook::cdecl_stub<void(rage::grcResourceCache*, void*)> grcResourceCache_queueDelete([]()
{
	return hook::get_call(hook::get_pattern("48 85 FF 74 0B 48 8B D7 48 8B C8 E8 ? ? ? ? 48 83 63 28 00", 11));
});

static hook::cdecl_stub<void(rage::grcResourceCache*)> grcResourceCache_flushQueue([]()
{
	return hook::get_call(hook::get_pattern("8B 45 07 4C 8B 17 44 8B 4D FB", -5));
});

rage::grcResourceCache* rage::grcResourceCache::GetInstance()
{
	return _getResourceCache();
}

void rage::grcResourceCache::QueueDelete(void* graphicsResource)
{
	return grcResourceCache_queueDelete(this, graphicsResource);
}

void rage::grcResourceCache::FlushQueue()
{
	grcResourceCache_flushQueue(this);
}

static hook::cdecl_stub<rage::grcRenderTarget*(int idx, const char* name, int usage3, int width, int height, int format, void* metadata, uint8_t a, rage::grcRenderTarget* last)> _createRenderTarget([]()
{
	return hook::get_pattern("4C 8B 55 6F 4D 85 D2 74 57 49 8B 02", -0x3D);
});

rage::grcRenderTarget* CreateRenderTarget(int idx, const char* name, int usage3, int width, int height, int format, void* metadata, uint8_t a, rage::grcRenderTarget* last)
{
	return _createRenderTarget(idx, name, usage3, width, height, format, metadata, a, last);
}
