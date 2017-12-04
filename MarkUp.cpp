#include "MarkUp.h"
#include "utility.h"

enum class Tag
{
	red = 0,
	blue,
	count,
};

static constexpr char* tagText[] =
{
	"red",
	"blue",
};

/*
static constexpr const Hash tagHashes[] =
{
	ConstHashString(tagText[0]),
	ConstHashString(tagText[1]),
};
static_assert(ARRAY_COUNT(tagText) == (int)Tag::count);
*/


void DrawRedText(sf::Text& drawText)
{
	drawText.setFillColor(sf::Color::Red);
}

void DrawBlueText(sf::Text& drawText)
{
	drawText.setFillColor(sf::Color::Blue);
}

const TagFunction tagFunctions[]
{
	&DrawRedText,
	&DrawBlueText,
};
static_assert(ARRAY_COUNT(tagFunctions) == (int)Tag::count);

/*
std::vector<sf::Text> ApplyMarkup(std::string& line, std::vector<Tag>& tagStack)
{
	// parse a string and find all the tags.
	auto index = line.find('<');
	if (index != std::string::npos)
	{
		auto endPosition = line.find('>', index);
		auto tagName = line.substr(index, endPosition);

	}
	
	return {};
}
*/