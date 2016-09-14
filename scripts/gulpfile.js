/*

ESP8266 file system builder with PlatformIO support

Copyright (C) 2016 by Xose Pérez &lt;xose dot perez at gmail dot com&gt;

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see &lt;http://www.gnu.org/licenses/&gt;.

*/

// ----------------------------------------------------------------------------- // File system builder //-----------------------------------------------------------------------------

const gulp = require('gulp');
const plumber = require('gulp-plumber');
const htmlmin = require('gulp-htmlmin');
const cleancss = require('gulp-clean-css');
const uglify = require('gulp-uglify');
const gzip = require('gulp-gzip');
const del = require('del');
const useref = require('gulp-useref');
const gulpif = require('gulp-if');
const inline = require('gulp-inline');
var inlinesource = require('gulp-inline-source');

const datadir = '../data/'
const sourcedir = '../www/'
//const examplesdir = ['../examples/Melvanimate-withESPManager/data', './examples/Melvanimate-example/data']


/* Clean destination folder */
gulp.task('clean', function() {
    return del([datadir + '*'], { force : true } );
});

/* Copy static files */
gulp.task('files', function() {
    return gulp.src([
            sourcedir + '**/*.{jpg,jpeg,png,ico,gif}',
            sourcedir + 'fsversion'
        ])
        .pipe(gulp.dest(datadir));
});

// gulp.task('copy', function() {
//    return gulp.src([datadir]).pipe(gulp.dest('output/folder', ));
// })

/* Process HTML, CSS, JS --- INLINE --- */
// gulp.task('inline', function() {
//     return gulp.src('html/*.{htm,html}')
//         .pipe(inline({
//             base: 'html/',
//             js: uglify,
//             css: cleancss,
//             disabledTypes: ['svg', 'img']
//         }))
//         .pipe(htmlmin({
//             collapseWhitespace: true,
//             removeComments: true,
//             minifyCSS: true,
//             minifyJS: false
//         }))
//         .pipe(gzip())
//         .pipe(gulp.dest('data'));
// })

/* Process HTML, CSS, JS */
// gulp.task('html', function() {
//     return gulp.src('html/*.{html,htm}')
//         .pipe(useref())
//         .pipe(plumber())
//         .pipe(gulpif('*.css', cleancss()))
//         .pipe(gulpif('*.js', uglify()))
//         .pipe(gulpif('*.html', htmlmin({
//             collapseWhitespace: true,
//             removeComments: true,
//             minifyCSS: true,
//             minifyJS: true
//         })))
//         .pipe(inline({
//             base: 'data.unminified/',
//             js: uglify,
//             css: cleancss
//         }))
//         .pipe(gzip())
//         .pipe(gulp.dest('data'));
// });

module.exports = function customjs(source, context, next) {
    if (source.fileContent &&
        !source.content &&
        (source.type == 'js')) {
        // The `inline-compress` attribute automatically overrides the global flag
        if (source.compress) {
            // compress content
        }
        if (source.props.foo == 'foo') {
            // foo content
        }
        next();
    } else {
        next();
    }
};

gulp.task('inlinesource', function() {

    var options = {
        compress: false

    };

    return gulp.src(sourcedir + '*.{html,htm}')
        .pipe(gulpif('*.{html,htm}', htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true
        })))
        .pipe(inlinesource(options))
        .pipe(gzip())
        .pipe(gulp.dest(datadir));
});


/* Build file system */
// gulp.task('buildfs', ['clean', 'files', 'html']);
// gulp.task('buildfs2', ['clean', 'files', 'inline']);
// gulp.task('default', ['buildfs']);
gulp.task('default', ['clean', 'files', 'inlinesource']);


// ----------------------------------------------------------------------------- // PlatformIO support //-----------------------------------------------------------------------------

const spawn = require('child_process').spawn;
const argv = require('yargs').argv;

var platformio = function(target) {
    var args = ['run'];
    if ("e" in argv) {
        args.push('-e');
        args.push(argv.e);
    }
    if ("p" in argv) {
        args.push('--upload-port');
        args.push(argv.p);
    }
    if (target) {
        args.push('-t');
        args.push(target);
    }
    const cmd = spawn('platformio', args);
    cmd.stdout.on('data', function(data) {
        console.log(data.toString().trim());
    });
    cmd.stderr.on('data', function(data) {
        console.log(data.toString().trim());
    });
}

gulp.task('uploadfs', ['buildfs'], function() {
    platformio('uploadfs');
});
gulp.task('upload', function() {
    platformio('upload');
});
gulp.task('run', function() {
    platformio(false);
});
