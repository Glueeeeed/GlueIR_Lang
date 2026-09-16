import { defineConfig } from 'vitepress'

export default defineConfig({
  title: "GlueIR Lang",
  description: "Documentation for GlueIR, a minimalist, procedural programming language based on LLVM",
  lang: 'en-US',

  themeConfig: {
    nav: [
      { text: 'Home', link: '/' },
      { text: 'Guide', link: '/guide/getting-started' },
      { text: 'Architecture', link: '/internals/compiler-architecture' },
      { text: 'Examples', link: '/examples/code-samples' }
    ],

    sidebar: {
      '/guide/': [
        {
          text: 'Language Guide',
          items: [
            { text: 'Getting Started & Installation', link: '/guide/getting-started' },
            { text: 'Syntax, Types & Mutability', link: '/guide/syntax-and-types' },
            { text: 'Control Flow & Conditions', link: '/guide/control-flow' },
            { text: 'Functions & I/O', link: '/guide/functions-and-io' }
          ]
        }
      ],
      '/internals/': [
        {
          text: 'Compiler Internals',
          items: [
            { text: 'Architecture & LLVM Pipeline', link: '/internals/compiler-architecture' }
          ]
        }
      ],
      '/examples/': [
        {
          text: 'Code Samples',
          items: [
            { text: 'Sample Programs', link: '/examples/code-samples' }
          ]
        }
      ]
    },

    socialLinks: [
      { icon: 'github', link: 'https://github.com/glueeeeed/GlueIR_Lang' }
    ],

    footer: {
      message: 'Released under Open Source.',
      copyright: 'Copyright © 2026 Glueeed'
    },

    search: {
      provider: 'local'
    }
  }
})
