# Configuration file for the Sphinx documentation builder.
# doc: https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information

project = "Mark's DevOps 雜碎"
copyright = '2022, Mark Zhu'
author = 'Mark Zhu'

release = '0.1'
version = '0.1.0'

# -- General configuration

extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.intersphinx',
    'sphinx.ext.autosectionlabel',
    'sphinx_rtd_theme',
    'myst_parser'
]

suppress_warnings = [
    'misc.highlighting_failure',
]

autosectionlabel_prefix_document = True

html_title = "Mark's DevOps 雜碎"
html_favicon = '_static/favicon.ico'
html_logo = "_static/logo.png"

html_theme_options = {
    # "home_page_in_toc": True,
    # "github_url": "https://github.com/labilezhu/devops-insider",
    # "repository_url": "https://github.com/labilezhu/devops-insider",
    # "repository_branch": "master",
    # "path_to_docs": "docs",
    # "use_repository_button": True,
    # "use_edit_page_button": False,
    # "show_navbar_depth": 8,
    "collapse_navigation": False,
    "logo_only": True,
    "navigation_depth": 8
}

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

myst_enable_extensions = [
    "html_image",
    "colon_fence"
]

templates_path = ['_templates']

# -- Options for HTML output

html_theme = 'sphinx_rtd_theme'
numfig = False
language = 'zh_CN'

# -- Options for EPUB output
epub_show_urls = 'footnote'

html_static_path = ["_static"]
html_css_files = ["custom.css"]
