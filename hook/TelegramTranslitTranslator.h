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

#ifndef TelegramTranslitTranslatorH
#define TelegramTranslitTranslatorH

#include <string>
#include <map>

namespace TelegramTranslit
{
    class Translator
    {
        //additional
    public:
        enum ActionType {Add, Replace, NoTranslation};
        struct NextAction
        {
            wchar_t characater;
            ActionType actionType;
        };

    private:
        typedef std::map<std::string, wchar_t> TranslitMap;

        //construction
    public:
        Translator();

        //services
    public:
        NextAction next(char c);

        //data
    public:
        std::string mCharSequence;
        const static TranslitMap mTranslitMap;
        
    }; //class Translator

} //namespace TelegramTranslit

#endif  //TelegramTranslitTranslatorH