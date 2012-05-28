; Drupal version
core = 7.x
api = 2

; Contrib modules
projects[context][subdir] = contrib
projects[context][version] = 3.0-beta2
projects[ctools][subdir] = contrib
projects[ctools][version] = 1.0
projects[debut][subdir] = contrib
projects[debut][version] = 1.0-rc1
projects[debut_media][subdir] = contrib
projects[debut_media][version] = 1.0-rc1
projects[features][subdir] = contrib
projects[features][version] = 1.0-rc2
projects[file_admin][subdir] = contrib
projects[file_admin][version] = 1.0-alpha1
projects[file_entity][subdir] = contrib
projects[file_entity][version] = 2.0-unstable4
projects[jcarousel][subdir] = contrib
projects[jcarousel][version] = 2.6
projects[media][subdir] = contrib
projects[media][version] = 2.0-unstable4
projects[media_browser_plus][subdir] = contrib
projects[media_browser_plus][version] = 1.0-beta3
projects[media_gallery][subdir] = contrib
projects[media_gallery][version] = 1.0-beta7
projects[media_vimeo][subdir] = contrib
projects[media_vimeo][version] = 1.0-beta5
projects[media_youtube][subdir] = contrib
projects[media_youtube][version] = 1.0-beta3
projects[multiform][subdir] = contrib
projects[multiform][version] = 1.0
projects[remote_stream_wrapper][subdir] = contrib
projects[remote_stream_wrapper][version] = 1.0-beta4
projects[styles][subdir] = contrib
projects[styles][version] = 2.0-alpha8
; Remove patch when http://drupal.org/node/1186624#comment-4699856 reaches
; stable, in next release after 2.0-alpha8.
projects[styles][patch][http://drupal.org/files/issues/1186624-styles-media-fields.patch] = http://drupal.org/files/issues/1186624-styles-media-fields.patch
projects[transliteration][subdir] = contrib
projects[transliteration][version] = 3.0
projects[views][subdir] = contrib
projects[views][version] = 3.3

; Libraries
libraries[colorbox][download][type] = "get"
libraries[colorbox][download][url] = "https://github.com/jackmoore/colorbox/zipball/v1.3.16"
libraries[colorbox][directory_name] = "colorbox"

