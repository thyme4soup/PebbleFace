#include <pebble.h>

Window *s_main_window;
Layer *s_circle_layer;
static GRect window_frame;
GRect bounds;
GPoint center;
GPoint center1;
GPoint center2;
GPoint center3;
double r;
double r1;
double r2;
double r3;

//math
static double floor(double d) {
  return (double) ((int) d);
}
static double ceil(double d) {
  double f = floor(d);
  if(d > f) return f + 1;
  return f;
}
static double intPow(double x, double pow) {
  double a = 1;
  for(int c = 0; c < pow; c++) {
    a *= x;
  }
  return a;
}
static double d_round(double x, unsigned int digits) {
  double fac = intPow(10.0, digits + 1);
  if(((int) (x * fac) % 10) > 5) {
    return ceil(x * fac) / fac;
  }
  else {
    return floor(x * fac) / fac;
  }
}

static void updateCenter1(int32_t t) {
  int denom = 60 * 60 * 12;
  t = t % denom;
  double tz = TRIG_MAX_ANGLE * ((double) t) / denom;
  double x = ((double) center.x) + (r1 - 1) * sin_lookup(tz) / TRIG_MAX_RATIO;
  double y = ((double) center.y) - (r1 - 1) * cos_lookup(tz) / TRIG_MAX_RATIO;
  center1.x = d_round(x, 0);
  center1.y = d_round(y, 0);
}
static void updateCenter2(int32_t t) {
  int denom = 60 * 60;
  t = t % denom;
  double tz = TRIG_MAX_ANGLE * t / denom;
  double x = ((double) center1.x) + (r2 - 1) * sin_lookup(tz) / TRIG_MAX_RATIO;
  double y = ((double) center1.y) - (r2 - 1) * cos_lookup(tz) / TRIG_MAX_RATIO;
  center2.x = d_round(x, 0);
  center2.y = d_round(y, 0);
}
static void updateCenter3(int32_t t) {
  int denom = 60;
  t = t % denom;
  double tz = TRIG_MAX_ANGLE * t / denom;
  double x = ((double) center2.x) + (r3 - 1) * sin_lookup(tz) / TRIG_MAX_RATIO;
  double y = ((double) center2.y) - (r3 - 1) * cos_lookup(tz) / TRIG_MAX_RATIO;
  center3.x = d_round(x, 0);
  center3.y = d_round(y, 0);
}

//graphics
static void circleDraw(GContext *ctx) {
  time_t now = time(NULL);
  struct tm *time = localtime(&now);
  int32_t t;
  t = time->tm_sec + time->tm_min * 60 + time->tm_hour * 60 * 60;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  //graphics_context_set_antialiased(ctx, true);
  //graphics_context_set_stroke_width(ctx, 3);
  //large outer circle
  graphics_draw_circle(ctx, center, r);
  graphics_draw_circle(ctx, center, r + 1);
  //medium inner circle (hour)
  updateCenter1(t);
  graphics_draw_circle(ctx, center1, r1);
  graphics_draw_circle(ctx, center1, r1 + 1);
  
  //small inner circle (minute)
  updateCenter2(t);
  graphics_draw_circle(ctx, center2, r2);
  graphics_draw_circle(ctx, center2, r2 + 1);
  
  //smallest circle(seconds)
  updateCenter3(t);
  graphics_draw_circle(ctx, center3, r3);
  graphics_draw_circle(ctx, center3, r3 + 1);
}

static void circle_update_callback(Layer *me, GContext *ctx) {
  circleDraw(ctx);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_change) {
  layer_mark_dirty(window_get_root_layer(s_main_window));
}


static void main_window_load() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect frame = window_frame = layer_get_frame(window_layer);
  
  s_circle_layer = layer_create(frame);
  layer_set_update_proc(s_circle_layer, circle_update_callback);
  layer_add_child(window_layer, s_circle_layer);
  
  //get bounds for circle constraints
  bounds = layer_get_bounds(window_layer);
  
  //init of circle parameters
  
  //just a couple things to keep track of
  int constraint;
  int min_border = 10;
  
  //set constraint to smallest length (x or y) of watch screen. (should be a square anyways, but y'know how we do)
  if(bounds.size.w <= bounds.size.h) constraint = bounds.size.w;
  else constraint = bounds.size.h;
  
  //radii initialized
  r = (constraint - min_border * 2) / 2; //todo, handle for off-center by one pixel
  r1 = r/2;
  r2 = r1/2;
  r3 = r2/2;
  
  //init center of the universe
  center = GPoint((bounds.size.w - 2*r) / 2 + r, (bounds.size.h - 2*r) / 2 + r);
  
  //abstract initial location assigned, will be changed during first iteration
  center1 = GPoint(r1, 100);
  center2 = GPoint(r2, 100);
  center3 = GPoint(r3, 100);
}

static void main_window_unload() {
  layer_destroy(s_circle_layer);
}

static void init(void) {
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  
  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  
  tick_timer_service_unsubscribe();
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
