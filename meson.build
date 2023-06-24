project(
    'co_sample',
    'cpp',
    default_options: ['cpp_std=c++latest'],
)
subdir('simple')
subdir('awaiter')
subdir('generator')
subdir('ret')
subdir('task')

asio_dep = dependency('asio')
subdir('asio_http_get')
subdir('asio_simple')

