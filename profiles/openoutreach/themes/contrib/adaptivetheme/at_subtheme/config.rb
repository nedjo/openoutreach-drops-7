#
# This file is only needed for Compass/Sass integration. If you are not using
# Compass, you may safely ignore or delete this file.
#
# If you'd like to learn more about Sass and Compass, see the sass/_README.txt
# file for more information.


# Directory paths
# -----------------------------------------------------------------------------
css_dir = "css"
sass_dir = "sass"
images_dir = "css/images"


# Add imports
# -----------------------------------------------------------------------------
# Assuming this theme is in sites/*/themes/THEMENAME, you can add the partials
# included with a module by uncommenting and modifying one of the lines below:
#add_import_path "../../../default/modules/FOO"
#add_import_path "../../../all/modules/FOO"
#add_import_path "../../../../modules/FOO"


# Precision
# -----------------------------------------------------------------------------
Sass::Script::Number.precision = 5


# Environment
# -----------------------------------------------------------------------------
environment = :development
#environment = :production


# Output Style
# -----------------------------------------------------------------------------
# You can select your preferred output style here (:expanded, :nested, :compact
# or :compressed). :expanded is closest to Drupal coding standards and it is
# not necessary to compress in the preprocessor since Drupal will do this for
# us using its own aggregation and compression systems.
output_style = :expanded


# Assets
# -----------------------------------------------------------------------------
relative_assets = true


# Line Comments
# -----------------------------------------------------------------------------
line_comments = (environment == :development) ? true : false


# Sourcemaps
# -----------------------------------------------------------------------------
sourcemap = (environment == :development) ? true : false
