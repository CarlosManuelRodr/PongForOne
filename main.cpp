#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cmath>
#include <random>

// Audio
sf::Music music;
sf::SoundBuffer hit_sound;
sf::Sound sound;

// Ventana
unsigned window_width = 800;
unsigned window_height = 600;

// Varios
float friction_coefficient = 0.05;
sf::Text score_label;
int score = 0;

template <class N> std::string num_to_string(const N d)
{
    std::ostringstream oss;
    oss << d;
    return oss.str();
}

class Ball
{
public:
    sf::Texture texture;
    sf::Sprite shape;
    sf::Vector2f position;
    sf::Vector2f speed;
    sf::Vector2u size;

    Ball()
    {
    	// Cargar textura
        if(!texture.loadFromFile("resources/ball.png"))
            std::cout << "Error: No se pudo cargar textura de la ball" << std::endl;
        else
            shape.setTexture(texture);

        size = texture.getSize();

        // Generar posicion inicial aleatoria
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> uni(0, window_width - size.x);

        position = sf::Vector2f(uni(rng), 0.0f);
        speed = sf::Vector2f(200.0f, 200.0f);
        shape.setPosition(position);
    }
    void Move(float deltatime)
    {
        // Maneja movimiento y colisiones con el mundo
        if (position.x < 0)
        {
            position.x = 0;
            speed.x = -speed.x;
        }
        if ((position.x + size.x) > window_width)
        {
            position.x = window_width - size.x;
            speed.x = -speed.x;
        }
        if (position.y < 0)
        {
            position.y = 0;
            speed.y = -speed.y;
        }

        position += deltatime * speed;
    }
    void Draw(sf::RenderWindow* target)
    {
        shape.setPosition(position);
        target->draw(shape);
    }
};

class Rectangle
{
public:
    sf::RectangleShape shape;
    sf::Vector2f position;
    sf::Vector2f size;
    float deltaspd;
    float speed;
    bool moving_left, moving_right;

    Rectangle()
    {
        position = sf::Vector2f(window_width/2-100, window_height-20);
        shape.setSize(sf::Vector2f(150, 20));
        shape.setPosition(position);
        shape.setFillColor(sf::Color::White);
        shape.setOutlineThickness(3);
        shape.setOutlineColor(sf::Color(250, 150, 100));

        size = shape.getSize();

        deltaspd = 0.2;
        moving_left = false;
        moving_right = false;
    }
    void Move(float deltatime)
    {
        // Procesasar cambios de velocidad
        if (moving_left)
            speed -= deltaspd;
        else if (speed < 0)
            speed += deltaspd;

        if (moving_right)
            speed += deltaspd;
        else if (speed > 0)
            speed -= deltaspd;


        // Maneja movimiento y colisiones con el mundo
        if (position.x < 0)
        {
            position.x = 0;
            speed = 0;
        }
        else if (position.x > window_width-size.x)
        {
            position.x = window_width-size.x;
            speed = 0;
        }
        else
            position.x += deltatime * speed;
    }
    void Draw(sf::RenderWindow* target)
    {
        shape.setPosition(position);
        target->draw(shape);
    }
    void ProcessEvent(sf::Event* event)
    {
    	// Cuando la tecla se presiona, activa el movimiento
        if (event->type == sf::Event::KeyPressed)
        {
            switch (event->key.code)
            {
            case sf::Keyboard::A:
                moving_left = true;
                break;
            case sf::Keyboard::Left:
                moving_left = true;
                break;
            case sf::Keyboard::D:
                moving_right = true;
                break;
            case sf::Keyboard::Right:
                moving_right = true;
                break;
            };
        }

        // Desactiva el movimiento cuando la tecla se deja de presionar
        if (event->type == sf::Event::KeyReleased)
        {
            switch (event->key.code)
            {
            case sf::Keyboard::A:
                moving_left = false;
                break;
            case sf::Keyboard::Left:
                moving_left = false;
                break;
            case sf::Keyboard::D:
                moving_right = false;
                break;
            case sf::Keyboard::Right:
                moving_right = false;
                break;
            };
        }
    }
};

void collision_handler(Ball* ball, Rectangle* rectangle)
{
    // Checar colisiones entre los objetos
    if ((ball->position.y + ball->size.x) >  (window_height - rectangle->size.y))
    {
        if ((ball->position.x > rectangle->position.x) && (ball->position.x < (rectangle->position.x + rectangle->size.x)))
        {
            // Aumentar dificultad
            ball->speed.y += 20;

            // Invertir velocidad y colocar
            ball->position.y = window_height - rectangle->size.y - ball->size.x - 1;
            ball->speed.y = -ball->speed.y;
            ball->speed.x += friction_coefficient * rectangle->speed;

            // Sonido de colision
            sound.play();

            // Actualizar marcador de posicion
            score += 1;
            score_label.setString(std::string("Score: ") + num_to_string(score));
        }
    }
}

bool end_game(Ball* ball)
{
	// Termina el juego cuando la bola sale de la pantalla por abajo
    if ((ball->position.y) > window_height)
    {
        music.stop();
        return true;
    }
    return false;
}

int main()
{
    // Crear la ventana
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Pong");

    // Abrir musica
    if (music.openFromFile("resources/Tetris_theme.ogg"))
        music.play();
    else
        std::cout << "Error: No se pudo abrir archivo de musica" << std::endl;

    // Sonido de colision
    if (hit_sound.loadFromFile("resources/hit.wav"))
        sound.setBuffer(hit_sound);
    else
        std::cout << "Error: No se pudo abrir archivo de audio" << std::endl;

    // Fondo
    sf::Texture background;
    sf::Sprite background_sprite;
    if(!background.loadFromFile("resources/background.png"))
        std::cout << "Error: No se pudo abrir imagen de fondo" << std::endl;
    else
        background_sprite.setTexture(background);
    bool eff_red_direction = false, eff_green_direction = false, eff_blue_direction = false;
    float eff_red = 50.0, eff_green = 50.0, eff_blue = 50.0;

    // Marcador de puntuacion
    sf::Font font;
    if (!font.loadFromFile("resources/DiavloFont.otf"))
        std::cout << "Error: No se pudo cargar fuente de texto" << std::endl;
    score_label.setFont(font);
    score_label.setString("Score: 0");
    score_label.setFillColor(sf::Color::White);
    score_label.setOutlineColor(sf::Color::Black);
    score_label.setOutlineThickness(4);
    score_label.setStyle(sf::Text::Bold);
    score_label.setCharacterSize(34);

    // Texto game over
    sf::Text game_over;
    game_over.setFont(font);
    game_over.setString("Game over");
    game_over.setCharacterSize(70);
    game_over.setPosition(220, 250);

    // Reloj
    sf::Clock clock;

    // Crear objetos
    Ball ball;
    Rectangle rectangle;

    // Loop principal
    while (window.isOpen())
    {
        // Procesar eventos
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            rectangle.ProcessEvent(&event);
        }

        // Limpiar pantalla y poner fondo negro
        window.clear(sf::Color::Black);

        if (!end_game(&ball))
        {
            // Manejador de tiempo
            sf::Time time = clock.getElapsedTime();
            clock.restart();

            // Mover objetos
            rectangle.Move(time.asSeconds());
            ball.Move(time.asSeconds());

            // Detectar colisiones
            collision_handler(&ball, &rectangle);

            // Dibujar objetos
            window.draw(background_sprite);
            rectangle.Draw(&window);
            ball.Draw(&window);
            window.draw(score_label);

            // Efectos de cambio de color de fondo
            background_sprite.setColor(sf::Color(eff_red, eff_green, eff_blue));
            if (eff_red_direction)
                eff_red += time.asSeconds()*30.0f;
            else
                eff_red -= time.asSeconds()*30.0f;

            if (eff_green_direction)
                eff_green += time.asSeconds()*50.0f;
            else
                eff_green -= time.asSeconds()*50.0f;

            if (eff_blue_direction)
                eff_blue += time.asSeconds()*70.0f;
            else
                eff_blue -= time.asSeconds()*70.0f;

            if (eff_red < 1 || eff_red > 200)
                eff_red_direction = !eff_red_direction;
            if (eff_green < 1 || eff_green > 200)
                eff_green_direction = !eff_green_direction;
            if (eff_blue < 1 || eff_blue > 200)
                eff_blue_direction = !eff_blue_direction;

        }
        else
        {
        	// Cuando el juego termina ya solo se dibuja el texto de game over y la puntuacion
            window.draw(game_over);
            window.draw(score_label);
        }

        // Terminar de dibujar
        window.display();
    }

    return 0;
}
