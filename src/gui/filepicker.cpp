//
// filepicker.cpp - A ROM chooser
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/22/2010  Created this file
//

#include "filepicker.h"

#include "crc32.h"
#include "filelistmodel.h"
#include "filethread.h"
#include "settings.h"
#include "types.h"

struct RomIdentifier
{
	const uint32 crc32;
	const char name[128];
	const char file[128];
};

RomIdentifier romList2[] = {
	{ 0x0509C85E, "Raiden (World)", "" },
	{ 0x08F15576, "Iron Soldier (World) (v1.04)", "" },
	{ 0x0957A072, "Kasumi Ninja (World)", "" },
	{ 0x0AC83D77, "NBA Jam T.E. (World)", "" },
	{ 0x0EC5369D, "Evolution - Dino Dudes (World)", "" },
	{ 0x0F6A1C2C, "Ultra Vortek (World)", "" },
	{ 0x14915F20, "White Men Can't Jump (World)", "" },
	{ 0x1660F070, "Power Drive Rally (World)", "" },
	{ 0x1E451446, "Trevor McFur in the Crescent Galaxy (World)", "" },
	{ 0x27594C6A, "Defender 2000 (World)", "" },
	{ 0x2E17D5DA, "Bubsy in Fractured Furry Tales (World)", "" },
	{ 0x348E6449, "Double Dragon V - The Shadow Falls (World)", "" },
	{ 0x3615AF6A, "Fever Pitch Soccer (World) (En,Fr,De,Es,It)", "" },
	{ 0x38A130ED, "Troy Aikman NFL Football (World)", "" },
	{ 0x3C044941, "Skyhammer (World)", "" },
	{ 0x42A13EC5, "Soccer Kid (World)", "" },
	{ 0x47EBC158, "Theme Park (World)", "" },
	{ 0x4899628F, "Hover Strike (World)", "" },
	{ 0x53DF6440, "Space War 2000 (World)", "" },
	{ 0x55A0669C, "[BIOS] Atari Jaguar Developer CD (World)", "" },
	{ 0x58272540, "Syndicate (World)", "" },
	{ 0x5A101212, "Sensible Soccer - International Edition (World)", "" },
	{ 0x5B6BB205, "Ruiner Pinball (World)", "" },
	{ 0x5CFF14AB, "Pinball Fantasies (World)", "" },
	{ 0x5E2CDBC0, "Doom (World)", "" },
	{ 0x61C7EEC0, "Zero 5 (World)", "" },
	{ 0x67F9AB3A, "Battle Sphere Gold (World)", "" },
	{ 0x687068D5, "[BIOS] Atari Jaguar CD (World)", "" },
	{ 0x6B2B95AD, "Tempest 2000 (World)", "" },
	{ 0x6EB774EB, "Worms (World)", "" },
	{ 0x6F8B2547, "Super Burnout (World)", "" },
	{ 0x817A2273, "Pitfall - The Mayan Adventure (World)", "" },
	{ 0x8975F48B, "Zool 2 (World)", "" },
	{ 0x8D15DBC6, "[BIOS] Atari Jaguar Stubulator '94 (World)", "" },
	{ 0x8FEA5AB0, "Dragon - The Bruce Lee Story (World)", "" },
	{ 0x97EB4651, "I-War (World)", "" },
	{ 0xA27823D8, "Ultra Vortek (World) (v0.94) (Beta)", "" },
	{ 0xA56D0798, "Protector - Special Edition (World)", "" },
	{ 0xA9F8A00E, "Rayman (World)", "" },
	{ 0xB14C4753, "Fight for Life (World)", "" },
	{ 0xBCB1A4BF, "Brutal Sports Football (World)", "" },
	{ 0xBDA405C6, "Cannon Fodder (World)", "" },
	{ 0xBDE67498, "Cybermorph (World) (Rev 1)", "" },
	{ 0xC5562581, "Zoop! (World)", "" },
	{ 0xC654681B, "Total Carnage (World)", "" },
	{ 0xC6C7BA62, "Fight for Life (World) (Alt)", "" },
	{ 0xC9608717, "Val d'Isere Skiing and Snowboarding (World)", "" },
	{ 0xCBFD822A, "Air Cars (World)", "" },
	{ 0xCD5BF827, "Attack of the Mutant Penguins (World)", "" },
	{ 0xD6C19E34, "Iron Soldier 2 (World)", "" },
	{ 0xDA9C4162, "Missile Command 3D (World)", "" },
	{ 0xDC187F82, "Alien vs Predator (World)", "" },
	{ 0xDE55DCC7, "Flashback - The Quest for Identity (World) (En,Fr)", "" },
	{ 0xE28756DE, "Atari Karts (World)", "" },
	{ 0xE60277BB, "[BIOS] Atari Jaguar Stubulator '93 (World)", "" },
	{ 0xE91BD644, "Wolfenstein 3D (World)", "" },
	{ 0xEC22F572, "SuperCross 3D (World)", "" },
	{ 0xECF854E7, "Cybermorph (World) (Rev 2)", "" },
	{ 0xEEE8D61D, "Club Drive (World)", "" },
	{ 0xF0360DB3, "Hyper Force (World)", "" },
	{ 0xFA7775AE, "Checkered Flag (World)", "" },
	{ 0xFAE31DD0, "Flip Out! (World)", "" },
	{ 0xFB731AAA, "[BIOS] Atari Jaguar (World)", "" },
	{ 0xFFFFFFFF, "***END***", "" }
};

/*
Our strategy here is like so:
Look at the files in the directory pointed to by ROMPath.
For each file in the directory, take the CRC32 of it and compare it to the CRC
in the romList[]. If there's a match, put it in a list and note it's index value
in romList for future reference.

When constructing the list, use the index to pull up an image of the cart and
put that in the list. User picks from a graphical image of the cart.

Ideally, the label will go into the archive along with the ROM image, but that's
for the future...
Maybe box art, screenshots will go as well...
*/

//FilePickerWindow::FilePickerWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog)//could use Window as well...
FilePickerWindow::FilePickerWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Window)
{
	setWindowTitle("Insert Cartridge...");

#if 1
	fileList2 = new QListWidget(this);
//	addWidget(fileList);

	QVBoxLayout * layout = new QVBoxLayout();
//	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	layout->addWidget(fileList2);

//	PopulateList();
	fileThread = new FileThread(this);

	/*bool b =*/ connect(fileThread, SIGNAL(FoundAFile(unsigned long)), this, SLOT(AddFileToList(unsigned long)));
//printf("FilePickerWindow: Connection to FileThread %s...\n", (b ? "succeeded" : "failed"));

	fileThread->Go(fileList2);
#else
QStringList numbers;
numbers << "One" << "Two" << "Three" << "Four" << "Five";

QAbstractItemModel * model = new StringListModel(numbers);
QListView * view = new QListView;
view->setModel(model);

#endif
}

// Need a slot here to pickup stuff from the thread...

void FilePickerWindow::AddFileToList(unsigned long index)
{
	printf("--> Found CRC: %08X...\n", (uint32)index);
}


/*
void FilePickerWindow::PopulateList(void)
{
	QDir romDir(vjs.ROMPath);
	QFileInfoList list = romDir.entryInfoList();

	for(int i=0; i<list.size(); i++)
	{
		QFileInfo fileInfo = list.at(i);
//         std::cout << qPrintable(QString("%1 %2").arg(fileInfo.size(), 10)
//                                                 .arg(fileInfo.fileName()));
//         std::cout << std::endl;
		QFile file(romDir.filePath(fileInfo.fileName()));
		uint8 * buffer = new uint8[fileInfo.size()];

		if (file.open(QIODevice::ReadOnly))
		{
			file.read((char *)buffer, fileInfo.size());
			uint32 crc = crc32_calcCheckSum(buffer, fileInfo.size());
			file.close();
//printf("FilePickerWindow: File crc == %08X...\n", crc);

			for(int j=0; romList2[j].crc32 != 0xFFFFFFFF; j++)
			{
				if (romList2[j].crc32 == crc)
				{
printf("FilePickerWindow: Found match [%s]...\n", romList2[j].name);
					new QListWidgetItem(QIcon(":/res/generic.png"), romList2[j].name, fileList);
					break;
				}
			}
		}

		delete[] buffer;
	}
}
*/