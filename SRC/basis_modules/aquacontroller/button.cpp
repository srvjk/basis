#include "button.h"
#include <iostream>

using namespace std;

std::map<std::string, std::shared_ptr<Button>> Button::_buttons;

Button::Button(const std::string& name)
{

}

std::shared_ptr<Button> Button::make(const std::string& name, sf::RenderWindow* window)
{
	std::shared_ptr<Button> btn = nullptr;

	auto iter = _buttons.find(name);
	if (iter != _buttons.end()) {
		btn = iter->second;
	}
	else {
		Button* pBtn = new Button(name);
		btn = std::shared_ptr<Button>(pBtn);
		_buttons[name] = btn;
	}

	btn->processEvents(window);

	return btn;
}

void Button::processEvents(sf::RenderWindow* window)
{
	//cout << "processEvents" << endl;

	_clicked = false;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2i pos = sf::Mouse::getPosition(*window);
		sf::FloatRect bounds = _rectShape.getGlobalBounds();
		if (bounds.contains(sf::Vector2f(pos)))
			_state = State::Pressed;
	}
	else {
		sf::Vector2i pos = sf::Mouse::getPosition(*window);
		sf::FloatRect bounds = _rectShape.getGlobalBounds();
		if (bounds.contains(sf::Vector2f(pos))) {
			if (_state == State::Pressed) {
				_clicked = true;
				_state = State::None;
			}
		}
	}
}

void Button::draw(sf::RenderWindow* window)
{
	window->draw(_rectShape);
	window->draw(_text);

	if (_state == State::Pressed) {
		sf::RectangleShape rect;
		rect.setPosition(_rectShape.getPosition());
		rect.setSize(_rectShape.getSize());
		rect.setFillColor(sf::Color(10, 10, 10, 100));
		window->draw(rect);
	}
}

void Button::setRect(const sf::FloatRect& rect)
{
	_rectShape.setPosition(sf::Vector2f(rect.left, rect.top));
	_rectShape.setSize(sf::Vector2f(rect.width, rect.height));
}

void Button::setBkColor(const sf::Color& color)
{
	_rectShape.setFillColor(color);
}

void Button::setText(const sf::Text& text)
{
	_text = text;
	_text.setPosition(_rectShape.getPosition());
}

bool Button::clicked() const
{
	return _clicked;
}

