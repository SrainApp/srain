#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Srain documentation build configuration file, created by
# sphinx-quickstart on Sun Jul 16 12:29:59 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

# -- Enviroment information -----------------------------------------------------

CI = os.environ.get('CI') is not None

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.githubpages',
    'sphinx.ext.intersphinx',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['.templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = 'Srain'
copyright = '2022, Shengyu Zhang'
author = 'Shengyu Zhang'
baseurl = 'https://srain.silverrainz.me/'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = '1.5.1'
# The full version, including alpha/beta/rc tags.
release = version

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = 'en'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = ['.build', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
# HTML theme configuration
if CI:
    html_theme = 'sphinx_book_theme'
else:
    html_theme = 'alabaster'

if CI:
    html_theme_options = {
        'repository_url': 'https://github.com/SrainApp/srain',
        "use_repository_button": True,
        "use_download_button": False,
        "show_toc_level": 2,
    }

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_css_files = []

html_baseurl = baseurl

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
if CI:
    blog_post_page = [ # Provided by sphinx_book_theme
                      'sidebar-logo.html', 'search-field.html',
                      # Provided by ABlog
                      'postcard.html', 'recentposts.html', 'tagcloud.html',
                      'categories.html', 'archives.html']
    html_sidebars = {
        'blog': blog_post_page, # ABlog's "All Posts" page
        'blog/**': blog_post_page, # Inlucde posts and autogenerated pages
        'changelog': blog_post_page, # Inlucde posts and autogenerated pages
    }

html_logo = '../data/icons/hicolor/128x128/apps/im.srain.Srain.png'
html_favicon = html_logo

# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'Sraindoc'


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',

    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'Srain.tex', 'Srain Documentation',
     'Shengyu Zhang', 'manual'),
]


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'srain', 'Srain Documentation',
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'Srain', 'Srain Documentation',
     author, 'Srain', 'One line description of project.',
     'Miscellaneous'),
]

# -- Options for sphinx.ext.extlinks --------------------------------------

extensions.append('sphinx.ext.extlinks')

extlinks = {
    'issue': ('https://github.com/SrainApp/srain/issues/%s', '#%s'),
    'pull': ('https://github.com/SrainApp/srain/pull/%s', '#%s'),
    'commit': ('https://github.com/SrainApp/srain/commit/%s', '%s'),

    'contrib-issue': ('https://github.com/SrainApp/contrib/issues/%s', 'contrib#%s'),
    'contrib-pull': ('https://github.com/SrainApp/contrib/pull/%s', 'contrib#%s'),
    'contrib-commit': ('https://github.com/SrainApp/srain/commit/%s', '%s'),

    'people': ('https://github.com/%s', '@%s'),
    'ghrepo': ('https://github.com/%s', '⛺ %s'),
}

if CI:
    extensions.append('sphinxnotes.strike')

    extensions.append('ablog')
    blog_path = 'blog'
    blog_title = project
    blog_baseurl = baseurl
    blog_authors = {
        author: (author, blog_baseurl),
    }
    blog_languages = {
        language: ('English',  None),
    }
    blog_default_author = author
    blog_default_language = language
    post_auto_image = 1
    blog_feed_fulltext = True
    fontawesome_included = True
    html_css_files.append('ablog-custom.css')

    extensions.append('sphinxnotes.mock')
    mock_directives = []
    mock_directives.append('contents') # Theme has built-in local-toc, see html_theme

    extensions.append('sphinx_sitemap')
    sitemap_filename = "sitemap.xml"
    sitemap_url_scheme = "{link}"

    extensions.append('sphinxcontrib.gtagjs')
    gtagjs_ids = ['G-6DC73T8933']

    extensions.append('sphinxext.opengraph')
    ogp_site_url = baseurl
    ogp_site_name = project