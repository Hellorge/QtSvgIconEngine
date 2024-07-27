#include "SvgIcon.h"
// https://github.com/eyllanesc/stackoverflow/blob/master/questions/50550089/main.cpp
SvgIcon::SvgIcon(const QPixmap &pixmap)
    : QObject(nullptr), QIcon(pixmap)
{
}

SvgIcon::~SvgIcon()
{
}
