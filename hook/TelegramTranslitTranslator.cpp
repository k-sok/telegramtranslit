// **********************************************************************************
// The MIT License(MIT)
//
// Copyright(c) 2015 Konstantin Sokolov <konstantin.sokolov.mail@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// **********************************************************************************

//header
#include "TelegramTranslitTranslator.h"

//standard
#include <cassert>

using namespace TelegramTranslit;
using namespace std;

//static member initialization
const Translator::TranslitMap Translator::mTranslitMap = {
    // small letters
    {"a", L'а'},
    {"b", L'б'},
    {"v", L'в'},
    {"g", L'г'},
    {"d", L'д'},
    {"e", L'е'},
    {"jo", L'ё'}, {"yo", L'ё'}, {"ö", L'ё'},
    {"zh", L'ж'},
    {"z", L'з'},
    {"i", L'и'},
    {"j", L'й'},
    {"k", L'к'},
    {"l", L'л'},
    {"m", L'м'},
    {"n", L'н'},
    {"o", L'о'},
    {"p", L'п'},
    {"r", L'р'},
    {"s", L'с'},
    {"t", L'т'},
    {"u", L'у'},
    {"f", L'ф'},
    {"h", L'х'}, {"x", L'х'},
    {"c", L'ц'},
    {"ch", L'ч'},
    {"sh", L'ш'},
    {"shh", L'щ'}, {"w", L'щ'},
    {"#", L'ъ'},
    {"y", L'ы'},
    {"'", L'ь'},
    {"je", L'э'}, {"ä", L'э'},
    {"ju", L'ю'}, {"yu", L'ю'}, {"ü", L'ю'},
    {"ja", L'я'}, {"ya", L'я'}, {"q", L'я'},

    // capital letters
    {"A", L'А'},
    {"B", L'Б'},
    {"V", L'В'},
    {"G", L'Г'},
    {"D", L'Д'},
    {"E", L'Е'},
    {"Jo", L'Ё'}, {"Yo", L'Ё'}, {"Ö", L'Ё'},
    {"Zh", L'Ж'},
    {"Z", L'З'},
    {"I", L'И'},
    {"J", L'Й'},
    {"K", L'К'},
    {"L", L'Л'},
    {"M", L'М'},
    {"N", L'Н'},
    {"O", L'О'},
    {"P", L'П'},
    {"R", L'Р'},
    {"S", L'С'},
    {"T", L'Т'},
    {"U", L'У'},
    {"F", L'Ф'},
    {"H", L'Х'}, {"X", L'Х'},
    {"C", L'Ц'},
    {"Ch", L'Ч'},
    {"Sh", L'Ш'},
    {"Shh", L'Щ'}, {"W", L'Щ'},
    {"##", L'Ъ'},
    {"Y", L'Ы'},
    {"\"", L'Ь'},
    {"Je", L'Э'}, {"Ä", L'Э'},
    {"Ju", L'Ю'}, {"Yu", L'Ю'}, {"Ü", L'Ю'},
    {"Ja", L'Я'}, {"Ya", L'Я'}, {"Q", L'Я'},
};

//construction
Translator::Translator() {}

//services
Translator::NextAction Translator::next(char c)
{   
    wchar_t resultCharacter = 0;
    ActionType actionType = ActionType::NoTranslation;

    mCharSequence += c;
    TranslitMap::const_iterator it = mTranslitMap.find(mCharSequence);

    if (it == mTranslitMap.end())
    {
        mCharSequence = string(1, c);
        it = mTranslitMap.find(mCharSequence);

        if (it == mTranslitMap.end())
        {
            mCharSequence.clear();
        }
        else
        {
            assert(mCharSequence.size() == 1);
            resultCharacter = it->second;
            actionType = ActionType::Add;
        }
    }
    else
    {
        actionType = mCharSequence.size() > 1 ? ActionType::Replace : ActionType::Add;
        resultCharacter = it->second;
    }

    return Translator::NextAction{resultCharacter, actionType};
}
