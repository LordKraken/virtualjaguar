//
// keygrabber.cpp - Widget to grab a key and dismiss itself
//
// by James L. Hammons
// (C) 2011 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  07/18/2011  Created this file
//

#include "keygrabber.h"


KeyGrabber::KeyGrabber(QWidget * parent/*= 0*/): QDialog(parent)
{
	label = new QLabel(this);
	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->addWidget(label);
	setLayout(mainLayout);
	setWindowTitle(tr("Grab"));

	// Will this make Mac OSX work???
	setFocusPolicy(Qt::StrongFocus);
}

KeyGrabber::~KeyGrabber()
{
}

//void KeyGrabber::SetText(QString keyText)
void KeyGrabber::SetKeyText(int keyNum)
{
	char jagButtonName[21][10] = { "Up", "Down", "Left", "Right",
		"*", "7", "4", "1", "0", "8", "5", "2", "#", "9", "6", "3",
		"A", "B", "C", "Option", "Pause" };

	QString text = QString(tr("Press key for \"%1\"<br>(ESC to cancel)"))
		.arg(QString(jagButtonName[keyNum]));
	label->setText(text);
}

void KeyGrabber::keyPressEvent(QKeyEvent * e)
{
	key = e->key();

	// Since this is problematic, we don't allow this key...
	if (key != Qt::Key_Alt)
		accept();
}
