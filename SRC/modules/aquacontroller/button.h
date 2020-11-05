#pragma once

#include "basis.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

class Button
{
	enum class State {
		None,
		Pressed
	};
private:
	Button(const std::string& name);

public:
	void processEvents(sf::RenderWindow* window);
	void draw(sf::RenderWindow* window);
	void setRect(const sf::FloatRect& rect);
	void setBkColor(const sf::Color& color);
	void setText(const sf::Text& text);
	bool clicked() const;

public:
	static std::shared_ptr<Button> make(const std::string& name, sf::RenderWindow* window);

private:
	static std::map<std::string, std::shared_ptr<Button>> _buttons;
	State _state = State::None;
	bool _clicked = false;
	sf::RectangleShape _rectShape;
	sf::Text _text;
};
