#define LCD_HRES 320
#define LCD_VRES 240

#include <Arduino.h>
#include <lcd_controller.h>
#include <gfx.hpp>
#include <uix.hpp>
using namespace gfx;
using namespace uix;
// downloaded from fontsquirrel.com and header generated with
// https://honeythecodewitch.com/gfx/generator
#include <fonts/Telegrama.hpp>
#include <bee_icon.hpp>
static const open_font& text_font = Telegrama;

template<typename PixelType,typename PaletteType = gfx::palette<PixelType,PixelType>>
class bee_box : public control<PixelType,PaletteType> {
    using base_type = control<PixelType,PaletteType>;
    svg_doc m_doc;
    bool m_initialized;
public:

    bee_box(invalidation_tracker& parent) : base_type(parent),  m_initialized(false) {

    }
    void on_paint(control_surface<PixelType,PaletteType>& surface,const srect16& clip) {
        if(!m_initialized) {
            bee_icon.seek(0);
            gfx_result res = svg_doc::read(&bee_icon,&m_doc);
            if(gfx_result::success!=res) {
                Serial.printf("Error reading SVG: %d\n",(int)res);
            } else {
                m_initialized = true;
            }
        }
        if(m_initialized) {
            Serial.println("render svg");
            draw::svg(surface,this->dimensions().bounds(),m_doc,m_doc.scale(this->dimensions()),&clip);
        }
    }
};

using screen_t = screen<LCD_HRES, LCD_VRES, rgb_pixel<16>>;
using label_t = label<typename screen_t::pixel_type>;
using svg_box_t = bee_box<typename screen_t::pixel_type>;
using color_t = color<typename screen_t::pixel_type>;
using color32_t = color<rgba_pixel<32>>;
//using button_a_t = int_button<PIN_BUTTON_A, 10, true>;
//using button_b_t = int_button<PIN_BUTTON_B, 10, true>;
constexpr static const int lcd_buffer_size = 32 * 1024;
uint8_t lcd_buffer1[lcd_buffer_size];
uint8_t lcd_buffer2[lcd_buffer_size];
esp_lcd_panel_handle_t lcd_handle;

svg_doc doc;
//button_a_t button_a;
//button_b_t button_b;
screen_t main_screen(sizeof(lcd_buffer1), lcd_buffer1, lcd_buffer2);
label_t test_label(main_screen);
svg_box_t test_svg(main_screen);

static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    main_screen.set_flush_complete();
    return true;
}
static void uix_flush(point16 location, typename screen_t::bitmap_type& bmp, void* state) {
    int x1 = location.x, y1 = location.y, x2 = location.x + bmp.dimensions().width-1, y2 = location.y + bmp.dimensions().height-1;
    lcd_flush(x1,y1,x2,y2,bmp.begin());
}

void lcd_panel_init() {
    lcd_color_trans_done_register_cb(lcd_flush_ready,nullptr);
    lcd_init(sizeof(lcd_buffer1));
}

void screen_init() {
    test_label.bounds(srect16(spoint16(10, 10), ssize16(200, 60)));
    test_label.text_color(color32_t::blue);
    test_label.text_open_font(&text_font);
    test_label.text_line_height(50);
    test_label.text_justify(uix_justify::center);
    test_label.round_ratio(NAN);
    test_label.padding({8, 8});
    test_label.text("Hello");
    // make the backcolor transparent
    auto bg = color32_t::black;
    bg.channel<channel_name::A>(0);
    test_label.background_color(bg);
    // and the border
    test_label.border_color(bg);

    test_svg.bounds(srect16(spoint16(10,70),ssize16(200,60)));
    
    main_screen.background_color(color_t::white);
    main_screen.register_control(test_label);
    main_screen.register_control(test_svg);
    main_screen.on_flush_callback(uix_flush);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Booted");
    Serial.write(bee_icon_data,sizeof(bee_icon_data));
    Serial.println();
    //fs.close();
    lcd_panel_init();
    screen_init();
   
}
void loop() {

    main_screen.update();
}