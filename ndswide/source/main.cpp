#include <nds.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

// https://github.com/nayuki/QR-Code-generator
#include "BitBuffer.hpp"
#include "QrCode.hpp"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using std::uint8_t;

int frameCount = 0;
int lastFrameCount = 0;

void Vblank()
{
	frameCount++;
}

void OnKeyPressed(int key) {
   if(key > 0)
      iprintf("%c", key);
}

void wait(int count)
{
	while (frameCount - lastFrameCount < count)
	{
		swiWaitForVBlank();
	}
	lastFrameCount = frameCount;
}

void waitKey()
{
	scanKeys();
	while ((keysDown() & KEY_A) == 0)
	{
		swiWaitForVBlank();
		scanKeys();
	}
}

void fillRect(u16 *videoMemory, int x, int y, int w, int h, u16 c)
{
	for (int dx = 0; dx < w; dx++)
	{
		for (int dy = 0; dy < h; dy++)
		{
			videoMemory[x + dx + (y + dy) * 256] = c;
		}
	}
}

void dumpQR(u16 *videoMemoryMain, int blockid, uint8_t *buf, int len)
{
	std::vector<uint8_t> vec(4 + len);
	vec[0] = blockid & 0xff;
	vec[1] = (blockid >> 8) & 0xff;
	vec[2] = (blockid >> 16) & 0xff;
	vec[3] = (blockid >> 24) & 0xff;
	std::copy(buf, buf + len, vec.begin() + 4);
	std::vector<QrSegment> segs{QrSegment::makeEci(27), QrSegment::makeBytes(vec)};
	int mask = 0; // fix mask for speed
	const QrCode qr = QrCode::encodeSegmentsWide(segs, mask);

	int w = qr.getWidth(), h = qr.getHeight();
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int c = qr.getModule(x, y) ? 0 : 31;
			fillRect(videoMemoryMain, 2 * x + 2, 2 * y + 2, 2, 2, ARGB16(1, c, c, c));
		}
	}
	fillRect(videoMemoryMain, 2, 2, 4, 4, ARGB16(1, 8, 16, 29));
	fillRect(videoMemoryMain, 256 - 6, 2, 4, 4, ARGB16(1, 3, 26, 10));
	fillRect(videoMemoryMain, 256 - 6, 192 - 6, 4, 4, ARGB16(1, 28, 20, 7));
	fillRect(videoMemoryMain, 2, 192 - 6, 4, 4, ARGB16(1, 28, 11, 21));
}

QrCode makeQR(int blockid, uint8_t *buf, int len)
{
	std::vector<uint8_t> vec(4 + len);
	vec[0] = blockid & 0xff;
	vec[1] = (blockid >> 8) & 0xff;
	vec[2] = (blockid >> 16) & 0xff;
	vec[3] = (blockid >> 24) & 0xff;
	std::copy(buf, buf + len, vec.begin() + 4);
	std::vector<QrSegment> segs{QrSegment::makeEci(27), QrSegment::makeBytes(vec)};
	int mask = 0; // fix mask for speed
	return QrCode::encodeSegmentsWide(segs, mask);
}

void dumpQR2(u16 *videoMemoryMain, int blockid1, int blockid2, uint8_t *buf1, uint8_t *buf2, int len)
{
	QrCode qr1 = makeQR(blockid1, buf1, len);
	QrCode qr2 = makeQR(blockid2, buf2, len);
	int w = qr1.getWidth(), h = qr1.getHeight();
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int c1 = qr1.getModule(x, y) ? 0 : 31;
			int c2 = qr2.getModule(x, y) ? 0 : 31;
			int c3 = c1 & c2;
			fillRect(videoMemoryMain, 2 * x + 2, 2 * y + 2, 2, 2, ARGB16(1, c1, c2, c3));
		}
	}
	fillRect(videoMemoryMain, 2, 2, 4, 4, ARGB16(1, 8, 16, 29));
	fillRect(videoMemoryMain, 256 - 6, 2, 4, 4, ARGB16(1, 3, 26, 10));
	fillRect(videoMemoryMain, 256 - 6, 192 - 6, 4, 4, ARGB16(1, 28, 20, 7));
	fillRect(videoMemoryMain, 2, 192 - 6, 4, 4, ARGB16(1, 28, 11, 21));
}

const int BLOCK_SIZE = 0x480;

void writeRandomData(std::vector<uint8_t> &data, uint32_t seed)
{
	data.resize(BLOCK_SIZE);
	for (int i = 0; i < BLOCK_SIZE / 4; i++)
	{
		data[i * 4 + 0] = seed & 0xff;
		data[i * 4 + 1] = (seed >> 8) & 0xff;
		data[i * 4 + 2] = (seed >> 16) & 0xff;
		data[i * 4 + 3] = (seed >> 24) & 0xff;
		seed = seed * 0x41c64e6d + 0x6073;
	}
}

void dump(const std::vector<std::pair<int, int>> &ranges)
{
	videoSetMode(MODE_5_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	int bgMain = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	u16 *videoMemoryMain = bgGetGfxPtr(bgMain);
	for (int i = 0; i < 256 * 256; i++)
		videoMemoryMain[i] = ARGB16(1, 31, 31, 31);
	std::vector<uint8_t> testdata;
	writeRandomData(testdata, 0xdeadbeef);
	dumpQR(videoMemoryMain, 0xffffffff, &testdata[0], BLOCK_SIZE);
	/*	std::vector<uint8_t> testdata2;
	writeRandomData(testdata2, 0xcafecafe);
	dumpQR2(videoMemoryMain, 0, 1, &testdata[0], &testdata2[0], BLOCK_SIZE); */
	printf("drawed test data. push A\n");
	waitKey();
	char name[13] = {};
	strncpy(name, (char *)0x080000A0, 12);
	printf("Target to dump: %s\n", name);
	wait(0);
	int numRanges = ranges.size();
	for (int n = 0; n < numRanges; n ++) {
		int start = ranges[n].first;
		int end = ranges[n].second;
		for (int i = start; i <= end; i ++) {
			printf("Dumping %05d...", i);
			dumpQR(videoMemoryMain, i, ((uint8_t *)GBAROM) + i * BLOCK_SIZE, BLOCK_SIZE);
			printf("done\n");
			scanKeys();
			if (keysDown() & KEY_START) {
				return;
			}
			wait(60 * 1);
		}
	}
	printf("Done!\n");
}

template <class Container>
void split(const std::string& str, Container& cont, char delim = ' ')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}

void parseRanges(const char *buf, std::vector<std::pair<int, int>> &ranges) {
	std::string str(buf);
	std::vector<std::string> rangeStrs;
	split(str, rangeStrs, ',');
	int num = rangeStrs.size();
	ranges.resize(num);
	for (int i = 0; i < num; i ++) {
		std::vector<std::string> vec;
		split(rangeStrs[i], vec, '-');
		if (vec.size() == 2) {
			ranges[i] = std::make_pair(std::stoi(vec[0]), std::stoi(vec[1]));
		} else {
			ranges[i] = std::make_pair(std::stoi(vec[0]), std::stoi(vec[0]));
		}
	}
}

int main(void)
{
	irqSet(IRQ_VBLANK, Vblank);
	consoleDemoInit();
	Keyboard *kbd = keyboardDemoInit();
	kbd->OnKeyPressed = OnKeyPressed;
	videoSetMode(MODE_FB0);
	vramSetBankA(VRAM_A_LCD);
	sysSetCartOwner(1);

	while (1) {
		char buf[256];
		std::vector<std::pair<int, int>> ranges;
		consoleClear();
		printf("Input range (e.g. 0-127)\n>");
		fgets(buf, 256, stdin);
		parseRanges(buf, ranges);
		dump(ranges);
	}

	return 0;
}